package org.opensc.agenda.modules.screens;

import java.util.List;
import org.apache.torque.criteria.Criteria;
import org.apache.torque.criteria.SqlEnum;
import org.apache.turbine.modules.screens.VelocitySecureScreen;
import org.apache.turbine.pipeline.PipelineData;
import org.apache.velocity.context.Context;
import org.opensc.agenda.om.Risorse;
import org.opensc.agenda.om.RisorsePeer;

public class Calendario extends VelocitySecureScreen
{
  private final boolean randomDemo = false;

  @Override
  protected void doBuildTemplate(PipelineData data, Context context)
     throws Exception
  {
    context.put("success", "Congratulations, it worked!");
    context.put("randomDemo", randomDemo);
    if(!randomDemo)
    {
      Criteria c = new Criteria();
      c.where(RisorsePeer.RISORSE_ID, 0, SqlEnum.GREATER_THAN);
      c.addAscendingOrderByColumn(RisorsePeer.RISORSE_ID);
      List<Risorse> lsRisorse = RisorsePeer.doSelect(c);
      context.put("calendari", lsRisorse);
    }
  }

  @Override
  protected boolean isAuthorized(PipelineData pipelineData)
     throws Exception
  {
    // use data.getACL()
    return true;
  }
}
