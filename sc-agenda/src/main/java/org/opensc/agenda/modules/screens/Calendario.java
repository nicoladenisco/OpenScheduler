package org.opensc.agenda.modules.screens;

import org.apache.turbine.modules.screens.VelocitySecureScreen;
import org.apache.turbine.pipeline.PipelineData;
import org.apache.velocity.context.Context;

public class Calendario extends VelocitySecureScreen
{
  @Override
  protected void doBuildTemplate(PipelineData data, Context context)
     throws Exception
  {
    context.put("success", "Congratulations, it worked!");
    context.put("randomDemo", "true");
  }

  @Override
  protected boolean isAuthorized(PipelineData pipelineData)
     throws Exception
  {
    // use data.getACL()
    return true;
  }
}
