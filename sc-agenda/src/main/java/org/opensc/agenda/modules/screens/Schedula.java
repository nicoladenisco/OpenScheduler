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

import org.apache.turbine.modules.screens.VelocitySecureScreen;
import org.apache.turbine.pipeline.PipelineData;
import org.apache.velocity.context.Context;

/**
 * Controllore per Schedula.vm.
 *
 * @author Nicola De Nisco
 */
public class Schedula extends VelocitySecureScreen
{
  @Override
  protected void doBuildTemplate(PipelineData pd, Context cntxt)
     throws Exception
  {
  }

  @Override
  protected boolean isAuthorized(PipelineData pipelineData)
     throws Exception
  {
    return true;
  }
}
