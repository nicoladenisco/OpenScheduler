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
package org.opensc.agenda.services.json;

import java.util.Map;
import org.apache.fulcrum.json.JsonService;
import org.apache.torque.om.ColumnAccessByName;
import org.json.JSONObject;

/**
 * Estensione del del servizio json.
 *
 * @author Nicola De Nisco
 */
public interface ExtendedJsonService extends JsonService
{
  /**
   * Serializza un oggetto di torque in formato json.
   * @param toPopulate oggetto json da popolare
   * @param obj oggetto torque da cui estrarre i dati
   * @param obj2json mappa nomejson/nometorque dei valori da esportare
   * @return oggetto json popolato
   */
  public JSONObject toJson(JSONObject toPopulate, ColumnAccessByName obj, Map<String, String> obj2json);

  /**
   * Legge un json e aggiorna oggetto di torque;
   * @param dati oggetto json con i dati
   * @param toPopulate oggetto torque da aggiornare
   * @param json2obj mappa nometorque/nomejson dei valori da importare
   * @return
   */
  public ColumnAccessByName fromJson(JSONObject dati, ColumnAccessByName toPopulate, Map<String, String> json2obj);

  /**
   * Processa richiesta proveniente dalla servlet.
   * @param method metodo HTTP
   * @param toPopulate oggetto json da popolare
   * @param sRequest nome della richiesta
   * @param params parametri della richiesta
   * @return oggetto json popolato
   */
  public JSONObject processRequest(String method, String sRequest, Map<String, Object> params, JSONObject toPopulate);
}
