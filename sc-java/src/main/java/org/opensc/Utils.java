/*
 * Copyright (C) 2026 Nicola De Nisco
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
package org.opensc;

import java.util.Arrays;
import java.util.List;
import java.util.Properties;
import java.util.StringTokenizer;
import java.util.stream.Collectors;

/**
 * Utilita.
 *
 * @author Nicola De Nisco
 */
public class Utils
{
  public static String Properties2String(Properties properties)
  {
    StringBuilder rv = new StringBuilder(128);
    properties.forEach((k, v) -> rv.append(k).append('=').append(v).append('|'));
    return rv.toString();
  }

  public static Properties String2Properties(String pipeMap, Properties rv)
  {
    StringTokenizer stok = new StringTokenizer(pipeMap, "|");
    while(stok.hasMoreTokens())
    {
      String s = stok.nextToken();
      if(s.isEmpty())
        continue;
      int pos = s.indexOf('=');
      if(pos != -1)
      {
        String key = s.substring(0, pos);
        String val = s.substring(pos + 1);
        rv.put(key, val);
      }
    }
    return rv;
  }

  public static Properties String2Properties(String pipeMap)
  {
    return String2Properties(pipeMap, new Properties());
  }

  public static List<String> String2List(String pipeList)
  {
    return Arrays.asList(pipeList.split("\\|")).stream()
       .filter((s) -> !s.isEmpty())
       .collect(Collectors.toList());
  }
}
