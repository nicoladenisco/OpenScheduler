package org.opensc.agenda.services.pull;

import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.fulcrum.json.JsonService;
import org.apache.fulcrum.security.entity.Role;
import org.apache.fulcrum.security.model.turbine.TurbineAccessControlList;
import org.apache.fulcrum.security.util.RoleSet;
import org.apache.torque.criteria.Criteria;
import org.apache.torque.criteria.SqlEnum;
import org.apache.turbine.om.security.User;
import org.apache.turbine.services.TurbineServices;
import org.apache.turbine.services.pull.RunDataApplicationTool;
import org.apache.turbine.services.security.SecurityService;
import org.apache.turbine.util.RunData;
import org.commonlib5.utils.ArrayOper;
import org.json.JSONArray;
import org.json.JSONObject;
import org.opensc.agenda.om.Eventi;
import org.opensc.agenda.om.EventiPeer;
import org.opensc.agenda.om.Prestazioni;
import org.opensc.agenda.om.PrestazioniPeer;
import org.opensc.agenda.om.Risorse;
import org.opensc.agenda.om.RisorsePeer;
import org.opensc.agenda.services.json.ExtendedJsonServiceImpl;

public class RelatedTool implements RunDataApplicationTool
{
  /** Logging */
  private static final Log log = LogFactory.getLog(RelatedTool.class);

  private RunData data = null;

  @Override
  public void init(Object data)
  {
    this.data = (RunData) data;
  }

  @Override
  public void refresh(RunData data)
  {
    this.data = data;
  }

  public String getUserrole()
  {
    try
    {
      User user = data.getUser();
      Role role = null;
      if(user != null && user.getName() != null)
      {
        log.info("reading role for: " + user.getName());
        if(data.getACL() != null && data.getACL() instanceof TurbineAccessControlList)
        {
          RoleSet roles = ((TurbineAccessControlList) data.getACL()).getRoles();
          if(roles != null && roles.getSet().size() == 1)
          {
            Role fulcrumRole = roles.getSet().iterator().next();
            log.debug("acl role is: " + fulcrumRole.getName());
            return fulcrumRole.getName();
          }
        }
        else
        {
          SecurityService securityService = (SecurityService) TurbineServices.getInstance().getService(SecurityService.SERVICE_NAME);
          RoleSet roles = securityService.getAllRoles();
        }
      }
      else if(user != null)
      {
        SecurityService securityService = (SecurityService) TurbineServices.getInstance().getService(SecurityService.SERVICE_NAME);
        role = securityService.getRoleByName("user");
        //.retrieveRole("anon");
      }
      return (role != null) ? role.getName() : null;
    }
    catch(Exception e)
    {
      log.error("RelatedTool - failure in reading role: ", e);
      return null;
    }
  }

  @SuppressWarnings("unchecked")
  public <T> Object getJson(Object src, String className, String mixinCN, Boolean refresh, String... props)
  {
    String result = null;
    ExtendedJsonServiceImpl jsonService = (ExtendedJsonServiceImpl) TurbineServices
       .getInstance().getService(JsonService.ROLE);

    try
    {
      log.info("refresh is:" + refresh);
      log.info("jsonService:" + jsonService);
      log.info("source class is:" + className);
      log.info("target object is:" + src);
      Class clazz = Class.forName(className);

      if(props != null)
      {
        log.info("props length:" + props.length);
        for(int i = 0; i < props.length; i++)
        {
          log.debug("props:" + props[i]);
        }
      }

      if(mixinCN != null)
      {
        Class mixin = Class.forName(mixinCN);
        if(mixin != null)
        {
          Set<Class> mixins = new HashSet<>();
          mixins.add(mixin);
          log.info("adding adapter mixinCN:" + mixinCN);
          jsonService.addAdapter(mixinCN, clazz, mixin);
        }
      }

      String serialized = jsonService.serializeOnlyFilter(src, clazz, refresh, props);
      log.debug("serialized:" + serialized);
      return serialized;
    }
    catch(Exception e)
    {
      log.error(e.getMessage(), e);
      result = e.getMessage();
    }
    return result;
  }

  public String getRisorseJson()
  {
    try
    {
      ExtendedJsonServiceImpl jsonService = (ExtendedJsonServiceImpl) TurbineServices
         .getInstance().getService(JsonService.ROLE);

      Map<String, String> obj2json = ArrayOper.asMapFromPairStrings(
         "id", "RisorseId",
         "name", "Descrizione",
         "color", "Color",
         "borderColor", "Bordercolor",
         "backgroundColor", "Backgroundcolor",
         "dragBackgroundColor", "Dragbackgroundcolor",
         "", ""
      );

      Criteria c = new Criteria();
      c.where(RisorsePeer.RISORSE_ID, 0, SqlEnum.GREATER_THAN);
      c.addAscendingOrderByColumn(RisorsePeer.RISORSE_ID);
      List<Risorse> lsRes = RisorsePeer.doSelect(c);
      JSONArray rv = new JSONArray();

      for(Risorse r : lsRes)
      {
        rv.put(jsonService.toJson(new JSONObject(), r, obj2json));
      }

      return rv.toString();
    }
    catch(Exception e)
    {
      log.error(e.getMessage(), e);
      return e.getMessage();
    }
  }

  public String getEventiJson()
  {
    try
    {
      ExtendedJsonServiceImpl jsonService = (ExtendedJsonServiceImpl) TurbineServices
         .getInstance().getService(JsonService.ROLE);

      Map<String, String> obj2json = ArrayOper.asMapFromPairStrings(
         "id", "EventiId",
         "calendarId", "IdCalendar",
         "title", "Title",
         "body", "Body",
         "isReadOnly", "Isreadonly",
         "isPrivate", "Isprivate",
         "location", "Elocation",
         "attendees", "Attendees",
         "recurrenceRule", "Recurrencerule",
         "state", "Estate",
         "goingDuration", "Goingduration",
         "comingDuration", "Comingduration",
         "raw", "Eraw",
         "category", "Category",
         "start", "Startdate",
         "end", "Enddate",
         "", ""
      );

      Criteria c = new Criteria();
      c.where(EventiPeer.EVENTI_ID, 0, SqlEnum.GREATER_THAN);
      c.addAscendingOrderByColumn(EventiPeer.EVENTI_ID);
      List<Eventi> lsRes = EventiPeer.doSelect(c);
      JSONArray rv = new JSONArray();

      for(Eventi r : lsRes)
      {
        rv.put(jsonService.toJson(new JSONObject(), r, obj2json));
      }

      return rv.toString();
    }
    catch(Exception e)
    {
      log.error(e.getMessage(), e);
      return e.getMessage();
    }
  }

  public String prestazioniCombo()
  {
    try
    {
      Criteria c = new Criteria();
      c.where(PrestazioniPeer.PRESTAZIONI_ID, 0, SqlEnum.GREATER_THAN);
      c.addAscendingOrderByColumn(PrestazioniPeer.PRESTAZIONI_ID);
      List<Prestazioni> lsRes = PrestazioniPeer.doSelect(c);

      StringBuilder rv = new StringBuilder();
      for(Prestazioni p : lsRes)
        rv.append("<option value='").append(p.getCodice()).append("'>").append(p.getDescrizione()).append("</option>\n");
      return rv.toString();
    }
    catch(Exception e)
    {
      log.error(e.getMessage(), e);
      return e.getMessage();
    }
  }
}
