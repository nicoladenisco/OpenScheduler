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
package org.opensc.agenda.modules.screens;

import java.util.Date;
import org.apache.fulcrum.parser.ParameterParser;
import org.apache.turbine.modules.screens.VelocitySecureScreen;
import org.apache.turbine.pipeline.PipelineData;
import org.apache.velocity.context.Context;
import org.commonlib5.utils.DateTime;

/**
 * Controllore per Slots.vm.
 *
 * @author Nicola De Nisco
 */
public class Slots extends VelocitySecureScreen
{
  @Override
  protected void doBuildTemplate(PipelineData pd, Context ctx)
     throws Exception
  {
    ParameterParser pp = pd.getRunData().getParameters();
    String codPrest = pp.getString("prestazione");
    Date di = pp.getDate("di", DateTime.ISOformat);
    Date df = pp.getDate("df", DateTime.ISOformat);

    ctx.put("codPrest", codPrest);
    ctx.put("di", DateTime.formatIso(di));
    ctx.put("df", DateTime.formatIso(df));
  }

  @Override
  protected boolean isAuthorized(PipelineData pipelineData)
     throws Exception
  {
    return true;
  }
}
