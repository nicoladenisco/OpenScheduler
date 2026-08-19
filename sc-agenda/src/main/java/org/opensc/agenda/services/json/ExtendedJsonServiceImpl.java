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

import java.util.HashMap;
import java.util.Map;
import java.util.Set;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.fulcrum.json.jackson.Jackson2MapperService;
import org.apache.torque.om.ColumnAccessByName;
import org.commonlib5.utils.ClassOper;
import org.json.JSONObject;
import org.opensc.agenda.Utils;
import org.opensc.agenda.services.json.plugin.JsonPlugin;
import org.opensc.agenda.services.json.plugin.JsonPluginAnnotation;
import org.reflections.Reflections;

/**
 * Estensione del servizio turbine per la generazione di json.
 *
 * @author Nicola De Nisco
 */
public class ExtendedJsonServiceImpl extends Jackson2MapperService
   implements ExtendedJsonService
{
  /** Logging */
  private static final Log log = LogFactory.getLog(ExtendedJsonServiceImpl.class);

  protected final Reflections reflections = new Reflections("org.opensc.agenda.services.json.plugin");
  protected final Map<String, Class> annotati = new HashMap<>();
  protected final Map<String, Class> obsoleti = new HashMap<>();

  @Override
  public void initialize()
     throws Exception
  {
    super.initialize();
    cercaClassiAnnotate();

    // questo sta qua giusto per essere eseguito all'avvio
    Utils.initIDtable();
  }

  protected void cercaClassiAnnotate()
  {
    annotati.clear();
    obsoleti.clear();

    // Trova tutte le classi annotate con @JsonPluginAnnotation
    Set<Class<?>> annotatedClasses = reflections.getTypesAnnotatedWith(JsonPluginAnnotation.class);

    for(Class<?> clazz : annotatedClasses)
    {
      JsonPluginAnnotation annotation = clazz.getAnnotation(JsonPluginAnnotation.class);
      if(annotation == null)
        continue;

      String nome = annotation.nome();
      if(nome.isEmpty())
        continue;

      if("auto".equals(nome))
        nome = ClassOper.getClassName(clazz);

      if(annotation.obsoleto())
      {
        obsoleti.put(nome, clazz);
        continue;
      }

      annotati.put(nome, clazz);
    }
  }

  @Override
  public JSONObject toJson(JSONObject toPopulate, ColumnAccessByName obj, Map<String, String> obj2json)
  {
    for(Map.Entry<String, String> entry : obj2json.entrySet())
    {
      String jsonName = entry.getKey();
      String objName = entry.getValue();

      if(!objName.isEmpty())
      {
        Object value = obj.getByName(objName);
        if(value != null)
          toPopulate.put(jsonName, value);
      }
    }
    return toPopulate;
  }

  @Override
  public ColumnAccessByName fromJson(JSONObject dati, ColumnAccessByName toPopulate, Map<String, String> json2obj)
  {
    for(Map.Entry<String, String> entry : json2obj.entrySet())
    {
      String objName = entry.getKey();
      String jsonName = entry.getValue();

      if(!objName.isEmpty())
      {
        try
        {
          Object value = dati.opt(jsonName);
          if(value != null)
            toPopulate.setByName(objName, value);
        }
        catch(Exception ex)
        {
          log.error("setByName error: " + ex.getMessage());
        }
      }
    }
    return toPopulate;
  }

  @Override
  public JSONObject processRequest(String metod, String sRequest, Map<String, Object> params, JSONObject toPopulate)
  {
    Class<?> clazz = annotati.get(sRequest);
    if(clazz == null)
      throw new RuntimeException("Plugin " + sRequest + " non trovato.");

    try
    {
      JsonPlugin plugin = (JsonPlugin) clazz.getConstructor().newInstance();
      return plugin.processRequest(metod, sRequest, params, this, toPopulate);
    }
    catch(Exception ex)
    {
      throw new RuntimeException(ex);
    }
  }
}
