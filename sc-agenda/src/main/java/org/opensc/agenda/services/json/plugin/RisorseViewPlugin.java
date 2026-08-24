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
import java.sql.Connection;
import java.util.Map;
import org.apache.torque.Torque;
import org.commonlib5.utils.ArrayOper;
import org.json.JSONArray;
import org.json.JSONObject;
import org.opensc.agenda.services.json.ExtendedJsonService;

/**
 * Plugin dati visualizzazione risorse.
 *
 * @author Nicola De Nisco
 */
@JsonPluginAnnotation(nome = "risorseview")
public class RisorseViewPlugin implements JsonPlugin
{
  public static final Map<String, String> obj2json = ArrayOper.asMapFromPairStrings(
     "id", "risorse_id",
     "code", "codice",
     "name", "descrizione",
     "color", "color",
     "bordercolor", "bordercolor",
     "backgroundcolor", "backgroundcolor",
     "dragbackgroundcolor", "dragbackgroundcolor",
     "group", "gruppo"
  );

  @Override
  public JSONObject processRequest(String method, String sRequest, Map<String, Object> params,
     ExtendedJsonService service, JSONObject toPopulate)
     throws Exception
  {
    switch(method)
    {
      case "GET":
        return processRequestGET(sRequest, params, service, toPopulate);
//      case "POST":
//        return processRequestPOST(sRequest, params, service, toPopulate);
//      case "DELETE":
//        return processRequestDELETE(sRequest, params, service, toPopulate);

      default:
        throw new Exception("Metodo " + method + " non implementato.");
    }
  }

  /**
   * Lettura eventi.
   *
   * @param sRequest
   * @param params
   * @param service
   * @param toPopulate
   * @return
   * @throws Exception
   */
  protected JSONObject processRequestGET(String sRequest, Map<String, Object> params, ExtendedJsonService service,
     JSONObject toPopulate)
     throws Exception
  {
//    String codPrest = params.getOrDefault("codPrest", "2010-01-01").toString();
//    String inizio = params.getOrDefault("renderStart", "2010-01-01").toString();
//    String fine = params.getOrDefault("renderEnd", "2100-12-31").toString();
//
//    Date di = DateTime.inizioGiorno(dfIso.parse(inizio));
//    Date df = DateTime.fineGiorno(dfIso.parse(fine));

    String sSQL
       = "SELECT R.*,RL.gruppo\n"
       + " FROM prestazioni P \n"
       + "  INNER JOIN risorse_link RL ON P.prestazioni_id=RL.id_prestazioni\n"
       + "  INNER JOIN risorse R ON RL.id_risorse=R.risorse_id\n"
       + " WHERE P.codice=${codPrest}\n"
       + " ORDER BY R.codice\n"
       + "";

    JSONArray rv = new JSONArray();
    try(Connection conn = Torque.getConnection();
       QueryDataSetMacro qds = new QueryDataSetMacro(conn, sSQL, params))
    {
      for(Record r : qds)
      {
        rv.put(service.toJson(new JSONObject(), r, obj2json));
      }
    }

    toPopulate.put("risorse", rv);
    return toPopulate;
  }
}
