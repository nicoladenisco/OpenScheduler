/*
 * Copyright (C) 2026 Nicola De Nisco
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
package org.opensc;

import java.io.File;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.List;
import java.util.Properties;

/**
 * Avvio libreria.
 * Lo scopo è di fornire una interfaccia per il debug del codice nativo.
 *
 * @author Nicola De Nisco
 */
public class ScJava
{
  public static void main(String[] args)
  {
    System.out.println("OpenScheduler Java Interface - ver 0.0.1");
    loadNativeLibrary();

    try
    {
      String test = "";
      if(args.length > 0)
        test = args[0].trim().toUpperCase();

      switch(test)
      {
        case "RESOURCE":
          resourceTest();
          break;
      }
    }
    catch(Exception e)
    {
      e.printStackTrace();
    }
  }

  private static void resourceTest()
     throws Exception
  {
    File fres = new File("/tmp/oskjava/R001_2026.slot");
    Properties properties = new Properties();

    fres.delete();
    fres.getParentFile().mkdirs();

    properties.setProperty("codice", "R001");
    properties.setProperty("nomefile", fres.getAbsolutePath());
//    properties.setProperty("", "");
//    properties.setProperty("", "");
//    properties.setProperty("", "");
//    properties.setProperty("", "");
//    properties.setProperty("", "");

    System.out.println("TEST build !!!");
    try(SchedResource result = SchedResource.build(properties))
    {
      System.out.println(result.dumpHeader(properties));
      //System.out.println(result.dumpSlots(properties));
    }

    System.out.println("TEST getStamperAlgos !!!");
    List<String> result = SchedResource.getStamperAlgos();
    System.out.println("Algos: " + result + "\n");

    System.out.println("TEST stampResources daily simple !!!");
    try(SchedResource instance = new SchedResource(fres))
    {
      instance.stampResources("daily", properties);
      properties.setProperty("daystart", "0");
      properties.setProperty("daystop", "10");
      System.out.println(instance.dumpSlots(properties));
    }

    System.out.println("TEST stampResources daily hours !!!");
    properties.setProperty("hourmap", "9,10,11,12,13,14,15,16");
    try(SchedResource instance = new SchedResource(fres))
    {
      instance.stampResources("daily", properties);
      properties.setProperty("daystart", "0");
      properties.setProperty("daystop", "10");
      System.out.println(instance.dumpSlots(properties));
    }

    System.out.println("TEST stampResources daily hours !!!");
    properties.setProperty("hourmap", "9,10,11,12,13,14,15,16");
    try(SchedResource instance = new SchedResource(fres))
    {
      instance.stampResources("daily", properties);
      properties.setProperty("daystart", "0");
      properties.setProperty("daystop", "10");
      System.out.println(instance.dumpSlots(properties));
    }
  }

  protected static void loadNativeLibrary()
  {
    Path currentRelativePath = Paths.get("");
    String s = currentRelativePath.toAbsolutePath().toString();
    System.out.println("Current absolute path is: " + s);
    String toload = s.replace("sc-java", "sc-cpp/debug/liboskcore.so");
    System.load(toload);
  }
}
