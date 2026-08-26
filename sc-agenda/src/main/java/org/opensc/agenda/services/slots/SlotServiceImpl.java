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
import java.util.ArrayList;
import java.util.Calendar;
import java.util.GregorianCalendar;
import java.util.List;
import java.util.Properties;
import java.util.function.Function;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.torque.criteria.Criteria;
import org.apache.torque.criteria.SqlEnum;
import org.apache.turbine.services.BaseService;
import org.apache.turbine.services.InitializationException;
import org.commonlib5.utils.StringJoin;
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
    Criteria c = new Criteria();
    c.where(RisorsePeer.RISORSE_ID, 0, SqlEnum.GREATER_THAN);
    c.addAscendingOrderByColumn(RisorsePeer.CODICE);
    List<Risorse> lsRisorse = RisorsePeer.doSelect(c);

    for(Risorse r : lsRisorse)
    {
      String codice = r.getCodice();
      File fres = getFileRisorse(codice);

      if(!fres.exists())
        generaRisorse(codice, fres);
    }
  }

  @Override
  public File getFileRisorse(String codice)
  {
    return new File(dirSlots, codice + "_2026.slot");
  }

  protected void generaRisorse(String codice, File fres)
     throws Exception
  {
    Properties properties = new Properties();
    properties.setProperty("codice", codice);
    properties.setProperty("nomefile", fres.getAbsolutePath());

    log.info("TEST build " + codice + " !!!");
    try(SchedResource instance = SchedResource.build(properties))
    {
      log.info(instance.dumpHeader(properties));
      //log.info(result.dumpSlots(properties));

      switch(codice)
      {
        default:
          instance.stampResources("daily", properties);
          break;
        case "d1":
        case "i1":
          properties.setProperty("hourmap", "9,10,11,12,13,14,15,16");
          properties.setProperty("daymap", StringJoin.build(",").addObjects(giorniValidiAnno(null)).join());
          instance.stampResources("free", properties);
          break;
        case "d2":
        case "i2":
          properties.setProperty("hourmap", "10,11,12,13,14,15,16");
          properties.setProperty("daymap", StringJoin.build(",").addObjects(giorniDispari()).join());
          instance.stampResources("free", properties);
          break;
        case "d3":
        case "i3":
          properties.setProperty("hourmap", "9,10,11,12,13,14,15,16");
          properties.setProperty("daymap", StringJoin.build(",").addObjects(giorniPari()).join());
          instance.stampResources("free", properties);
          break;
      }
    }
  }

  private List<Integer> giorniValidiAnno(Function<Integer, Boolean> funTest)
  {
    List<Integer> rv = new ArrayList<>(365);
    Calendar cal = new GregorianCalendar();
    cal.set(Calendar.DAY_OF_YEAR, 1);

    for(int i = 0; i < 365; i++)
    {
      // imposta giorno e ricalcola calendario
      cal.set(Calendar.DAY_OF_YEAR, i + 1);
      cal.getTime();

      int giorno = cal.get(Calendar.DAY_OF_YEAR);
      int gs = cal.get(Calendar.DAY_OF_WEEK);

      // esclude sabati e domeniche
      if(gs == 1 || gs == 7)
        continue;

      int mese = cal.get(Calendar.MONTH) + 1;
      int gmese = cal.get(Calendar.DAY_OF_MONTH);
      // scarta festivita note
      if(mese == 1 && (gmese == 1 || gmese == 2 || gmese == 6))
        continue;
      if(mese == 12 && (gmese == 24 || gmese == 25 || gmese == 31))
        continue;

      // applica funzione custom di accettazione se richiesto
      if(funTest != null && !funTest.apply(giorno))
        continue;

      rv.add(giorno);
    }
    return rv;
  }

  private List<Integer> giorniPari()
  {
    return giorniValidiAnno((g) -> (g % 2) == 0);
  }

  private List<Integer> giorniDispari()
  {
    return giorniValidiAnno((g) -> (g % 2) == 1);
  }
}
