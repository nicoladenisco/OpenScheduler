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

import java.util.Map;
import org.json.JSONObject;
import org.opensc.agenda.services.json.ExtendedJsonService;

/**
 * Plugin per generazione json.
 *
 * @author Nicola De Nisco
 */
public interface JsonPlugin
{
  /**
   * Processa richiesta proveniente dalla servlet.
   * @param method metodo HTTP
   * @param toPopulate oggetto json da popolare
   * @param sRequest nome della richiesta
   * @param params parametri della richiesta
   * @param service riferimento al servizio
   * @return oggetto json popolato
   * @throws java.lang.Exception
   */
  public JSONObject processRequest(String method, String sRequest, Map<String, Object> params, ExtendedJsonService service, JSONObject toPopulate)
     throws Exception;
}
