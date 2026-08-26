/*
 * Copyright (C) 2026 The Apache Software Foundation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
package org.opensc.agenda.services.json.plugin;

import com.workingdogs.village.QueryDataSetMacro;
import com.workingdogs.village.Record;
import java.io.File;
import java.sql.Connection;
import java.util.Calendar;
import java.util.Date;
import java.util.GregorianCalendar;
import java.util.List;
import java.util.Map;
import java.util.Properties;
import org.apache.torque.Torque;
import org.apache.turbine.services.TurbineServices;
import org.commonlib5.utils.Classificatore;
import org.commonlib5.utils.DateTime;
import org.commonlib5.utils.StringJoin;
import org.commonlib5.utils.StringOper;
import org.json.JSONArray;
import org.json.JSONObject;
import org.opensc.SchedMerger;
import org.opensc.SchedResource;
import org.opensc.agenda.services.json.ExtendedJsonService;
import static org.opensc.agenda.services.json.plugin.EventiCalendarioPlugin.dfIso;
import static org.opensc.agenda.services.json.plugin.RisorseViewPlugin.obj2json;
import org.opensc.agenda.services.slots.IncrocioRisorse;
import org.opensc.agenda.services.slots.SlotService;

/**
 * Plugin per gestione slot.
 *
 * @author Nicola De Nisco
 */
@JsonPluginAnnotation(nome = "slot|slot-dispo")
public class SlotPlugin implements JsonPlugin
{
  public static final String[] giorniSettimana =
  {
    "", "domenica", "lunedi", "martedi", "mercoledi", "giovedi", "venerdi", "sabato"
  };

  @Override
  public JSONObject processRequest(String method, String sRequest, Map<String, Object> params,
     ExtendedJsonService service, JSONObject toPopulate)
     throws Exception
  {
    switch(method)
    {
      case "GET":
        switch(sRequest)
        {
          case "slot":
            return processRequestGETslots(sRequest, params, service, toPopulate);

          case "slot-dispo":
            return processRequestGETslotsDispo(sRequest, params, service, toPopulate);
        }

      // case "POST":
      // return processRequestPOST(sRequest, params, service, toPopulate);
      // case "DELETE":
      // return processRequestDELETE(sRequest, params, service, toPopulate);
      default:
        throw new Exception("Metodo " + method + " non implementato.");
    }
  }

  protected JSONObject processRequestGETslots(String sRequest, Map<String, Object> params, ExtendedJsonService service,
     JSONObject toPopulate)
     throws Exception
  {
    SlotService slsrv = (SlotService) TurbineServices.getInstance().getService(SlotService.SERVICE_NAME);

    String codPrest = params.getOrDefault("codPrest", "undefined").toString();
    String inizio = params.getOrDefault("renderStart", "2010-01-01").toString();
    String fine = params.getOrDefault("renderEnd", "2100-12-31").toString();

    Date di = DateTime.inizioGiorno(dfIso.parse(inizio));
    Date df = DateTime.fineGiorno(dfIso.parse(fine));

    String dys = getDayOfYearString(di);
    String dyf = getDayOfYearString(df);

    String sSQL
       = "SELECT R.*,RL.gruppo\n"
       + "  FROM prestazioni P \n"
       + "    INNER JOIN risorse_link RL ON P.prestazioni_id=RL.id_prestazioni\n"
       + "    INNER JOIN risorse R ON RL.id_risorse=R.risorse_id\n"
       + " WHERE P.codice=${codPrest}\n"
       + " ORDER BY R.codice\n"
       + "";

    JSONArray rvRisorse = new JSONArray();
    JSONArray rvIncroci = new JSONArray();
    Classificatore<String, Record> rgruppi = new Classificatore<>(
       (r) -> StringOper.okStr(r.getValue("gruppo").asString(), IncrocioRisorse.CHIAVE_NON_RAGGRUPPATE));

    try(Connection conn = Torque.getConnection();
       QueryDataSetMacro qds = new QueryDataSetMacro(conn, sSQL, params))
    {
      for(Record r : qds)
      {
        String codice = r.getValue("codice").asOkString();
        rgruppi.aggiungi(r);

        JSONObject jsonRisorsa = service.toJson(new JSONObject(), r, obj2json);

        File fres = slsrv.getFileRisorse(codice);
        Properties properties = new Properties();
        properties.setProperty("codice", codice);
        properties.setProperty("nomefile", fres.getAbsolutePath());
        try(SchedResource sr = new SchedResource(fres))
        {
          jsonRisorsa.put("slheader", sr.getInfoHeader());
          Properties pdump = new Properties(properties);
          pdump.setProperty("daystart", dys);
          pdump.setProperty("daystop", dyf);
          jsonRisorsa.put("sldump", sr.dumpSlots(pdump));
        }

        rvRisorse.put(jsonRisorsa);
      }
    }

    List<List<Record>> incrociRisorse = IncrocioRisorse.generaIncroci(rgruppi);
    for(List<Record> lr : incrociRisorse)
    {
      try(SchedMerger merger = new SchedMerger())
      {
        for(Record r : lr)
        {
          String codice = r.getValue("codice").asOkString();
          File fres = slsrv.getFileRisorse(codice);

          Properties pmerge = new Properties();
          pmerge.setProperty("codice", codice);
          pmerge.setProperty("nomefile", fres.getAbsolutePath());
          pmerge.setProperty("daystart", dys);
          pmerge.setProperty("daystop", dyf);
          merger.mergeResources("default", pmerge);
        }

        JSONObject jsonIncrocio = new JSONObject();
        Properties pdump = new Properties();
        pdump.setProperty("daystart", dys);
        pdump.setProperty("daystop", dyf);
        jsonIncrocio.put("sldump", merger.dumpSlots(pdump));
        jsonIncrocio.put("name",
           StringJoin.build("/").addObjectsEx(lr, (r) -> r.getValue("codice").asOkString()).join());
        jsonIncrocio.put("descr",
           StringJoin.build("/").addObjectsEx(lr, (r) -> r.getValue("descrizione").asOkString()).join());
        rvIncroci.put(jsonIncrocio);
      }
    }

    toPopulate.put("risorse", rvRisorse);
    toPopulate.put("incroci", rvIncroci);
    return toPopulate;
  }

  protected int getDayOfYear(Date d)
  {
    Calendar cal = new GregorianCalendar();
    cal.setTime(d);
    return cal.get(Calendar.DAY_OF_YEAR);
  }

  protected String getDayOfYearString(Date d)
  {
    return Integer.toString(getDayOfYear(d));
  }

  protected JSONObject processRequestGETslotsDispo(String sRequest, Map<String, Object> params,
     ExtendedJsonService service, JSONObject toPopulate)
     throws Exception
  {
    SlotService slsrv = (SlotService) TurbineServices.getInstance().getService(SlotService.SERVICE_NAME);

    String codPrest = params.getOrDefault("codPrest", "undefined").toString();
    String incrocio = params.getOrDefault("incrocio", "").toString();
    String inizio = params.getOrDefault("renderStart", "2010-01-01").toString();
    String fine = params.getOrDefault("renderEnd", "2100-12-31").toString();

    Date di = DateTime.inizioGiorno(dfIso.parse(inizio));
    Date df = DateTime.fineGiorno(dfIso.parse(fine));

    String dys = getDayOfYearString(di);
    String dyf = getDayOfYearString(df);

    String[] codRisorse = incrocio.split("\\/");
    if(incrocio.isEmpty() || codRisorse.length == 0)
      return new JSONObject();

    String sSQL
       = "SELECT R.*,RL.gruppo\n"
       + "  FROM prestazioni P \n"
       + "    INNER JOIN risorse_link RL ON P.prestazioni_id=RL.id_prestazioni\n"
       + "    INNER JOIN risorse R ON RL.id_risorse=R.risorse_id\n"
       + " WHERE P.codice=${codPrest}\n"
       + "   AND R.codice IN (" + StringJoin.joinForSQL(codRisorse) + ")\n"
       + " ORDER BY R.codice\n"
       + "";

    JSONArray rvRisorse = new JSONArray();
    JSONArray rvDisponibilita = new JSONArray();

    try(Connection conn = Torque.getConnection();
       QueryDataSetMacro qds = new QueryDataSetMacro(conn, sSQL, params);
       SchedMerger merger = new SchedMerger())
    {
      for(Record r : qds)
      {
        String codice = r.getValue("codice").asOkString();
        JSONObject jsonRisorsa = service.toJson(new JSONObject(), r, obj2json);

        File fres = slsrv.getFileRisorse(codice);
        // Properties properties = new Properties();
        // properties.setProperty("codice", codice);
        // properties.setProperty("nomefile", fres.getAbsolutePath());
        // try(SchedResource sr = new SchedResource(fres))
        // {
        // jsonRisorsa.put("slheader", sr.getInfoHeader());
        // Properties pdump = new Properties(properties);
        // pdump.setProperty("daystart", dys);
        // pdump.setProperty("daystop", dyf);
        // jsonRisorsa.put("sldump", sr.dumpSlots(pdump));
        // }

        Properties pmerge = new Properties();
        pmerge.setProperty("codice", codice);
        pmerge.setProperty("nomefile", fres.getAbsolutePath());
        pmerge.setProperty("daystart", dys);
        pmerge.setProperty("daystop", dyf);
        merger.mergeResources("default", pmerge);

        rvRisorse.put(jsonRisorsa);
      }

      Properties ih = merger.getInfoHeader();
      int oraIniziale = StringOper.parse(ih.get("oraIniziale"), 0);
      int slotOra = StringOper.parse(ih.get("slotOra"), 0);

      // cerca uno o piu slot
      Properties propFind = new Properties();
      propFind.put("numSlots", "2");
      List<String> risultato = merger.findFreeSlot(propFind);
      Calendar cal = new GregorianCalendar();
      for(String s : risultato)
      {
        String[] ss = s.split(",");
        if(ss.length < 2)
          continue;

        int giorno = StringOper.parse(ss[0], 0);
        int slot = StringOper.parse(ss[1], 0);
        cal.set(Calendar.DAY_OF_YEAR, giorno + 1);
        Date data = cal.getTime();
        int gs = cal.get(Calendar.DAY_OF_WEEK);

        JSONObject jsonRisultato = new JSONObject();
        jsonRisultato.put("giorno", giorno + 1);
        jsonRisultato.put("slot", slot);
        jsonRisultato.put("date", dfIso.format(data));
        jsonRisultato.put("gs", gs);
        jsonRisultato.put("gsn", giorniSettimana[gs]);

        // calcolo orario
        int minuto = (oraIniziale * 60) + ((60 * slot) / slotOra);
        int ora = minuto / 60;
        int min = minuto % 60;
        jsonRisultato.put("ore", ora);
        jsonRisultato.put("minuti", min);
        jsonRisultato.put("orario", String.format("%02d:%02d", ora, min));

        rvDisponibilita.put(jsonRisultato);
      }
    }

    toPopulate.put("risorse", rvRisorse);
    toPopulate.put("dispo", rvDisponibilita);
    return toPopulate;
  }

}
