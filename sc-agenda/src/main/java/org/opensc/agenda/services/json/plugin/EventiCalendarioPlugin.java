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

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.List;
import java.util.Map;
import org.apache.torque.criteria.Criteria;
import org.apache.torque.criteria.SqlEnum;
import org.commonlib5.utils.ArrayMap;
import org.commonlib5.utils.ArrayOper;
import org.commonlib5.utils.DateTime;
import org.commonlib5.utils.StringOper;
import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;
import org.opensc.agenda.Utils;
import org.opensc.agenda.om.Eventi;
import org.opensc.agenda.om.EventiPeer;
import org.opensc.agenda.services.json.ExtendedJsonService;

/**
 * plugin per gestione eventi.
 *
 * @author Nicola De Nisco
 */
@JsonPluginAnnotation(nome = "eventi")
public class EventiCalendarioPlugin implements JsonPlugin
{
  public static final SimpleDateFormat dfIso = new SimpleDateFormat("yyyy-MM-dd");

  public static final Map<String, String> obj2json = ArrayOper.asMapFromPairStrings(
     "id", "EventiId",
     "calendarId", "IdCalendar",
     "title", "Title",
     "body", "Body",
     "isReadOnly", "Isreadonly",
     "isPrivate", "Isprivate",
     "location", "Elocation",
     "attendees", "Attendees",
     "recurrenceRule", "Recurrencerule",
     "state", "Estate",
     "goingDuration", "Goingduration",
     "comingDuration", "Comingduration",
     "raw", "Eraw",
     "category", "Category",
     "start", "Startdate",
     "end", "Enddate",
     "", "");

  public static final Map<String, String> json2obj = StringOper.reverseMap(obj2json, new ArrayMap<>());

  @Override
  public JSONObject processRequest(String method, String sRequest, Map<String, Object> params,
     ExtendedJsonService service, JSONObject toPopulate)
     throws Exception
  {
    switch(method)
    {
      case "GET":
        return processRequestGET(sRequest, params, service, toPopulate);
      case "POST":
        return processRequestPOST(sRequest, params, service, toPopulate);
      case "DELETE":
        return processRequestDELETE(sRequest, params, service, toPopulate);

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
    String inizio = params.getOrDefault("renderStart", "2010-01-01").toString();
    String fine = params.getOrDefault("renderEnd", "2100-12-31").toString();

    Date di = DateTime.inizioGiorno(dfIso.parse(inizio));
    Date df = DateTime.fineGiorno(dfIso.parse(fine));

    Criteria c = new Criteria();
    c.where(EventiPeer.EVENTI_ID, 0, SqlEnum.GREATER_THAN);
    c.and(EventiPeer.STARTDATE, di, SqlEnum.GREATER_EQUAL);
    c.and(EventiPeer.ENDDATE, df, SqlEnum.LESS_EQUAL);
    c.addAscendingOrderByColumn(EventiPeer.EVENTI_ID);
    List<Eventi> lsRes = EventiPeer.doSelect(c);
    JSONArray rv = new JSONArray();

    for(Eventi r : lsRes)
    {
      rv.put(service.toJson(new JSONObject(), r, obj2json));
    }

    toPopulate.put("eventi", rv);
    return toPopulate;
  }

  /**
   * Crea o modifica evento.
   *
   * @param sRequest
   * @param params
   * @param service
   * @param toPopulate
   * @return
   * @throws Exception
   */
  protected JSONObject processRequestPOST(String sRequest, Map<String, Object> params, ExtendedJsonService service,
     JSONObject toPopulate)
     throws Exception
  {
    String eid = params.getOrDefault("eid", "").toString().trim();
    String cid = params.getOrDefault("cid", "").toString().trim();
    String jsondata = params.getOrDefault("jsondata", "").toString().trim();
    JSONObject dati = new JSONObject(jsondata);

    Eventi e;
    if(eid.isEmpty())
    {
      e = new Eventi();
      e.setBody("");
    }
    else
    {
      e = EventiPeer.retrieveByPK(StringOper.parse(eid, 0));
      int idRisorsa = StringOper.parse(cid, e.getIdCalendar());
      e.setIdCalendar(idRisorsa);
    }

    Date start = parseDateJson(dati, "start");
    if(start != null)
      e.setStartdate(start);
    Date end = parseDateJson(dati, "end");
    if(end != null)
      e.setEnddate(end);

    service.fromJson(dati, e, json2obj);
    e.save();

    toPopulate.put("eid", e.getEventiId());
    return toPopulate;
  }

  /**
   * Estrae dal JSON il campo indicato interpretandolo come data ISO 8601.
   *
   * @param dati oggetto json
   * @param key nome del campo
   * @return la data corrispondente oppure null se il campo non esiste o è vuoto
   */
  public Date parseDateJson(JSONObject dati, String key)
  {
    try
    {
      String val = dati.getJSONObject(key).getJSONObject("d").getString("d");
      return DateTime.parseDateIso8601(val, null);
    }
    catch(JSONException jSONException)
    {
      // ignora eccezione se il campo non è presente
      return null;
    }
  }

  /**
   * Cancellazione evento.
   *
   * @param sRequest
   * @param params
   * @param service
   * @param toPopulate
   * @return
   * @throws Exception
   */
  protected JSONObject processRequestDELETE(String sRequest, Map<String, Object> params, ExtendedJsonService service,
     JSONObject toPopulate)
     throws Exception
  {
    String eid = params.getOrDefault("eid", "").toString().trim();
    if(!eid.isEmpty())
    {
      String sSQL = "DELETE FROM eventi WHERE eventi_id=" + eid;
      Utils.executeStatement(sSQL);
    }

    return toPopulate;
  }
}
