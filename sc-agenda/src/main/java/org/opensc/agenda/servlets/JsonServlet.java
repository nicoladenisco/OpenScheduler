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
package org.opensc.agenda.servlets;

import java.io.IOException;
import java.io.PrintWriter;
import java.util.Map;
import javax.servlet.ServletConfig;
import javax.servlet.ServletException;
import javax.servlet.ServletRequest;
import javax.servlet.ServletResponse;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import org.apache.fulcrum.json.JsonService;
import org.apache.turbine.services.TurbineServices;
import org.json.JSONObject;
import org.opensc.agenda.Utils;
import org.opensc.agenda.services.json.ExtendedJsonService;

/**
 * Chiamate in formato Json per applicazione.
 *
 * @author Nicola De Nisco
 */
public class JsonServlet extends HttpServlet
{
  private ExtendedJsonService jsonService;

  @Override
  public void init(ServletConfig config)
     throws ServletException
  {
    super.init(config);

    jsonService = (ExtendedJsonService) TurbineServices
       .getInstance().getService(JsonService.ROLE);
  }

  /**
   * Processes requests for both HTTP <code>GET</code> and <code>POST</code> methods.
   * @param request servlet request
   * @param response servlet response
   * @throws ServletException if a servlet-specific error occurs
   * @throws IOException if an I/O error occurs
   */
  @Override
  public void service(ServletRequest request, ServletResponse response)
     throws ServletException, IOException
  {
    serviceJson((HttpServletRequest) request, (HttpServletResponse) response);
  }

  protected void serviceJson(HttpServletRequest request, HttpServletResponse response)
     throws ServletException, IOException
  {
    // estrae nome della richiesta
    String sRequest = request.getPathInfo().substring(1);
    Map<String, Object> params = Utils.getParMap(request);
    String method = request.getMethod();
    JSONObject rv = jsonService.processRequest(method, sRequest, params, new JSONObject());

    response.setContentType("text/json;charset=UTF-8");
    try(PrintWriter out = response.getWriter())
    {
      out.println(rv.toString());
    }
  }

  // <editor-fold defaultstate="collapsed" desc="HttpServlet methods. Click on the + sign on the left to edit the code.">
  /**
   * Returns a short description of the servlet.
   * @return a String containing servlet description
   */
  @Override
  public String getServletInfo()
  {
    return "Dati in formato json.";
  }// </editor-fold>

}
