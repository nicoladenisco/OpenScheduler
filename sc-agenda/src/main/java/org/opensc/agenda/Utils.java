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
package org.opensc.agenda;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.util.HashMap;
import java.util.Map;
import javax.servlet.http.HttpServletRequest;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.torque.Torque;
import org.commonlib5.utils.StringOper;
import static org.commonlib5.utils.StringOper.okStr;

/**
 * Utilita.
 *
 * @author Nicola De Nisco
 */
public class Utils
{
  /** Logging */
  private static final Log log = LogFactory.getLog(Utils.class);

  public static final String PERM_PAR_KEY = "PermanentParameterMap";
  public static final String SESSION_ID = "sessionId";
  public static final String QUERY_STRING = "queryString";
  public static final String PATH_INFO = "pathInfo";

  public static int executeStatementQuiet(String sSQL)
  {
    try
    {
      return executeStatement(sSQL);
    }
    catch(Exception e)
    {
      log.error("executeStatementQuiet: " + e.getMessage());
      return -1;
    }
  }

  public static int executeStatement(String sSQL)
     throws Exception
  {
    try(Connection conn = Torque.getConnection())
    {
      try(PreparedStatement stmt = conn.prepareStatement(sSQL))
      {
        return stmt.executeUpdate();
      }
    }
  }

  public static void initIDtable()
  {
    // awk '/table.+name/{print $2}' application-schema.xml
    //  name="PRESTAZIONI"
    //  name="RISORSE"
    //  name="RISORSE_LINK" no
    //  name="EVENTI"

    String sSQL1
       = "INSERT INTO id_table(\n"
       + "	id_table_id, table_name, next_id, quantity)\n"
       + "	VALUES ("
       + "(SELECT MAX(id_table_id)+1 FROM id_table),"
       + " 'TABELLA', 1, 1)";

    String sSQL2
       = "UPDATE id_table\n"
       + "	SET next_id=(SELECT MAX(PRIMARY)+1 FROM TABELLA), quantity=1\n"
       + "	WHERE table_name='TABELLA'";

    executeStatementQuiet(StringOper.strReplace(sSQL1, "TABELLA", "PRESTAZIONI", "PRIMARY", "PRESTAZIONI_ID"));
    executeStatementQuiet(StringOper.strReplace(sSQL1, "TABELLA", "RISORSE", "PRIMARY", "RISORSE_ID"));
    executeStatementQuiet(StringOper.strReplace(sSQL1, "TABELLA", "EVENTI", "PRIMARY", "EVENTI_ID"));

    executeStatementQuiet(StringOper.strReplace(sSQL2, "TABELLA", "PRESTAZIONI", "PRIMARY", "PRESTAZIONI_ID"));
    executeStatementQuiet(StringOper.strReplace(sSQL2, "TABELLA", "RISORSE", "PRIMARY", "RISORSE_ID"));
    executeStatementQuiet(StringOper.strReplace(sSQL2, "TABELLA", "EVENTI", "PRIMARY", "EVENTI_ID"));
  }

  public static Map<String, Object> getParMap(HttpServletRequest request)
  {
    HashMap<String, Object> htParam = new HashMap<>();

    // estrae i parametri della richiesta (anche i campi di input con nome della form)
    Map<String, String[]> parameterMap = request.getParameterMap();
    for(Map.Entry<String, String[]> entry : parameterMap.entrySet())
    {
      String name = entry.getKey();
      String[] value = entry.getValue();

      if(value == null || value.length == 0)
        continue;

      // se contiene un solo valore lo passa come tale, altrimenti passa l'array dei valori
      if(value.length == 1)
      {
        htParam.put(name, value[0]);
        htParam.put(name.toLowerCase(), value[0]);
      }
      else
      {
        htParam.put(name, value);
        htParam.put(name.toLowerCase(), value);
      }
    }

    // carica i parametri fissi
    htParam.putIfAbsent(SESSION_ID, request.getSession().getId());
    htParam.putIfAbsent(QUERY_STRING, okStr(request.getQueryString()));
    htParam.putIfAbsent(PATH_INFO, okStr(request.getPathInfo()));

    return htParam;
  }
}
