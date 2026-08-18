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
import org.apache.torque.Torque;

/**
 * Utilita per il db.
 *
 * @author Nicola De Nisco
 */
public class DbUtils
{
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
}
