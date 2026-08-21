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
package org.opensc.agenda.services.slots;

import java.io.File;
import java.util.List;
import java.util.Properties;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.torque.criteria.Criteria;
import org.apache.torque.criteria.SqlEnum;
import org.apache.turbine.services.BaseService;
import org.apache.turbine.services.InitializationException;
import org.opensc.SchedResource;
import org.opensc.agenda.om.Risorse;
import org.opensc.agenda.om.RisorsePeer;

/**
 * Servizio gestione slots.
 *
 * @author Nicola De Nisco
 */
public class SlotServiceImpl extends BaseService
   implements SlotService
{
  /** Logging */
  private static final Log log = LogFactory.getLog(SlotServiceImpl.class);

  protected File dirSlots;
  protected List<String> stamperAlgos;

  @Override
  public void init()
     throws InitializationException
  {
    super.init();

    try
    {
      loadNativeLibrary();

      dirSlots = new File("/tmp/osk-agenda");
      dirSlots.mkdirs();

      stamperAlgos = SchedResource.getStamperAlgos();
      log.info("Stamper algos: " + stamperAlgos + "\n");

      initSlos();
      setInit(true);
    }
    catch(Exception ex)
    {
      log.error("init failed", ex);
    }
  }

  protected void loadNativeLibrary()
  {
    String libreria = "/home/nicola/cvsgithub/OpenScheduler/sc-cpp/debug/liboskcore.so";
    System.load(libreria);
  }

  public void initSlos()
     throws Exception
  {
    List<Risorse> lsRisorse = RisorsePeer.doSelect(new Criteria().where(RisorsePeer.RISORSE_ID, 0, SqlEnum.GREATER_THAN));

    for(Risorse r : lsRisorse)
    {
      String codice = r.getCodice();
      File fres = getFileRisorse(codice);

      if(!fres.exists())
      {
        Properties properties = new Properties();
        properties.setProperty("codice", codice);
        properties.setProperty("nomefile", fres.getAbsolutePath());

        try(SchedResource instance = SchedResource.build(properties))
        {
          log.info("creato file slots:\n" + instance.dumpHeader(properties));

          instance.clearAllSlots(SchedResource.SLOT_UNAVAILABLE);
          instance.stampResources("daily", properties);
        }
      }
    }
  }

  @Override
  public File getFileRisorse(String codice)
  {
    return new File(dirSlots, codice + "_2026.slot");
  }
}
