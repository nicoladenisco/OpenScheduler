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

import java.util.List;
import java.util.Map;
import org.apache.torque.criteria.Criteria;
import org.commonlib5.utils.ArrayOper;
import org.json.JSONArray;
import org.json.JSONObject;
import org.opensc.agenda.om.PrestazioniPeer;
import org.opensc.agenda.om.Risorse;
import org.opensc.agenda.om.RisorseLinkPeer;
import org.opensc.agenda.om.RisorsePeer;
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
     "id", "RisorseId",
     "name", "Descrizione",
     "color", "Color",
     "borderColor", "Bordercolor",
     "backgroundColor", "Backgroundcolor",
     "dragBackgroundColor", "Dragbackgroundcolor",
     "code", "Codice",
     "", ""
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
    String codPrest = params.getOrDefault("codPrest", "2010-01-01").toString();
//    String inizio = params.getOrDefault("renderStart", "2010-01-01").toString();
//    String fine = params.getOrDefault("renderEnd", "2100-12-31").toString();
//
//    Date di = DateTime.inizioGiorno(dfIso.parse(inizio));
//    Date df = DateTime.fineGiorno(dfIso.parse(fine));

    Criteria c = new Criteria();
    c.where(PrestazioniPeer.CODICE, codPrest);
    c.addJoin(PrestazioniPeer.PRESTAZIONI_ID, RisorseLinkPeer.ID_PRESTAZIONI);
    c.addJoin(RisorseLinkPeer.ID_RISORSE, RisorsePeer.RISORSE_ID);
    List<Risorse> lsRes = RisorsePeer.doSelect(c);
    JSONArray rv = new JSONArray();

    for(Risorse r : lsRes)
    {
      rv.put(service.toJson(new JSONObject(), r, obj2json));
    }

    toPopulate.put("risorse", rv);
    return toPopulate;
  }
}
