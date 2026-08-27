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
    SchedResource.setDebugMode(1);

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

        case "MERGE":
        case "MERGER":
          mergerTest();
          break;

        case "BOOK":
        case "BOOKING":
          bookingTest();
          break;

        case "FIND":
        case "SEARCH":
          findTest();
          break;

        default:
          System.out.println("Aggiungere uno di RESOURCE, MERGE, BOOK");
          break;
      }
    }
    catch(Exception e)
    {
      e.printStackTrace();
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

    System.out.println("TEST getInfoHeader !!!");
    try(SchedResource instance = new SchedResource(fres))
    {
      Properties prop = instance.getInfoHeader();
      System.out.println("prop: " + prop + "\n");
    }

    System.out.println("TEST getStamperAlgos !!!");
    List<String> result = SchedResource.getStamperAlgos();
    System.out.println("Algos: " + result + "\n");

    System.out.println("TEST stampResources daily simple !!!");
    try(SchedResource instance = new SchedResource(fres))
    {
      instance.clearAllSlots(SchedResource.SLOT_UNAVAILABLE);
      instance.stampResources("daily", properties);
      Properties pdump = new Properties(properties);
      pdump.setProperty("daystart", "0");
      pdump.setProperty("daystop", "10");
      System.out.println(instance.dumpSlots(pdump));
    }

    System.out.println("TEST stampResources daily hours !!!");
    properties.setProperty("hourmap", "9,10,11,12,13,14,15,16");
    try(SchedResource instance = new SchedResource(fres))
    {
      instance.clearAllSlots(SchedResource.SLOT_UNAVAILABLE);
      instance.stampResources("daily", properties);
      Properties pdump = new Properties(properties);
      pdump.setProperty("daystart", "0");
      pdump.setProperty("daystop", "10");
      System.out.println(instance.dumpSlots(pdump));
    }

    System.out.println("TEST stampResources free hours !!!");
    properties.setProperty("hourmap", "9,10,11,12,13,14,15,16");
    properties.setProperty("daymap", "1, 3, 5, 7, 9");
    try(SchedResource instance = new SchedResource(fres))
    {
      instance.clearAllSlots(SchedResource.SLOT_UNAVAILABLE);
      instance.stampResources("free", properties);
      Properties pdump = new Properties(properties);
      pdump.setProperty("daystart", "0");
      pdump.setProperty("daystop", "10");
      System.out.println(instance.dumpSlots(pdump));
    }
  }

  private static void mergerTest()
     throws Exception
  {
    System.out.println("TEST getMergerAlgos !!!");
    List<String> result = SchedMerger.getMergerAlgos();
    System.out.println("Algos: " + result + "\n");

    buildMergerResultTest("R001");
    buildMergerResultTest("R002");
    buildMergerResultTest("R003");
    buildMergerResultTest("R004");
    buildMergerResultTest("R005");

    System.out.println("TEST merge !!!");
    try(SchedMerger merger = new SchedMerger())
    {
      Properties properties = new Properties();
      mergeResource(merger, "R001", properties);
      mergeResource(merger, "R002", properties);
      mergeResource(merger, "R003", properties);
      mergeResource(merger, "R004", properties);
      mergeResource(merger, "R005", properties);
    }
  }

  protected static void mergeResource(final SchedMerger merger, String codice, Properties properties)
     throws OscNativeException
  {
    System.out.println("Merge resource " + codice);
    properties.setProperty("codice", codice);
    properties.setProperty("nomefile", "/tmp/oskjava/" + codice + "_2026.slot");
    merger.mergeResources("default", properties);
    Properties pdump = new Properties(properties);
    pdump.setProperty("daystart", "0");
    pdump.setProperty("daystop", "10");
    System.out.println(merger.dumpSlots(pdump));
  }

  protected static void buildMergerResultTest(String codice)
     throws Exception
  {
    File fres = new File("/tmp/oskjava/" + codice + "_2026.slot");
    Properties properties = new Properties();

    fres.delete();
    fres.getParentFile().mkdirs();

    properties.setProperty("codice", codice);
    properties.setProperty("nomefile", fres.getAbsolutePath());
//    properties.setProperty("", "");
//    properties.setProperty("", "");
//    properties.setProperty("", "");
//    properties.setProperty("", "");
//    properties.setProperty("", "");

    System.out.println("TEST build " + codice + " !!!");
    try(SchedResource instance = SchedResource.build(properties))
    {
      System.out.println(instance.dumpHeader(properties));
      //System.out.println(result.dumpSlots(properties));

      switch(codice)
      {
        case "R001":
          instance.stampResources("daily", properties);
          break;
        case "R002":
          properties.setProperty("hourmap", "9,10,11,12,13,14,15,16");
          instance.stampResources("daily", properties);
          break;
        case "R003":
          properties.setProperty("hourmap", "13,14,15,16,17,18,19,20");
          instance.stampResources("daily", properties);
          break;
        case "R004":
          properties.setProperty("hourmap", "9,10,11,12,13,14,15,16");
          properties.setProperty("daymap", giorniDispari());
          instance.stampResources("free", properties);
          break;
        case "R005":
          properties.setProperty("hourmap", "9,10,11,12,13,14,15,16");
          properties.setProperty("daymap", giorniPari());
          instance.stampResources("free", properties);
          break;
      }
    }
  }

  private static String giorniDispari()
  {
    StringBuilder sb = new StringBuilder(512);
    for(int i = 1; i <= 365; i += 2)
    {
      if(i != 1)
        sb.append(",");
      sb.append(i);
    }
    return sb.toString();
  }

  private static String giorniPari()
  {
    StringBuilder sb = new StringBuilder(512);
    for(int i = 2; i <= 365; i += 2)
    {
      if(i != 2)
        sb.append(",");
      sb.append(i);
    }
    return sb.toString();
  }

  private static void bookingTest()
     throws Exception
  {
    System.out.println("TEST getMergerAlgos !!!");
    List<String> result = SchedMerger.getMergerAlgos();
    System.out.println("Algos: " + result + "\n");

    buildMergerResultTest("R001");
    buildMergerResultTest("R002");
    buildMergerResultTest("R003");
    buildMergerResultTest("R004");

    System.out.println("TEST booking !!!");
    try(SchedMerger merger = new SchedMerger())
    {
      Properties propMerge = new Properties();
      mergeResource(merger, "R001", propMerge);
      mergeResource(merger, "R002", propMerge);
      mergeResource(merger, "R003", propMerge);
      mergeResource(merger, "R004", propMerge);

      // prenota uno o piu slot
      Properties propBooking = new Properties();
      propBooking.put("numSlots", "2");
      merger.reserveSlot(0, 13, 999999, propBooking);

      System.out.println("RISULTATO (X):");
      Properties pdump = new Properties();
      pdump.setProperty("daystart", "0");
      pdump.setProperty("daystop", "10");
      System.out.println("== MERGER ==");
      System.out.println(merger.dumpSlots(pdump));
    }

    Properties pdump = new Properties();
    pdump.setProperty("daystart", "0");
    pdump.setProperty("daystop", "10");
    dumpResource("R001", pdump);
    dumpResource("R002", pdump);
    dumpResource("R003", pdump);
    dumpResource("R004", pdump);
  }

  private static void dumpResource(String codice, Properties pdump)
     throws Exception
  {
    File fres = new File("/tmp/oskjava/" + codice + "_2026.slot");
    Properties properties = new Properties();
    properties.setProperty("codice", codice);
    properties.setProperty("nomefile", fres.getAbsolutePath());

    try(SchedResource instance = new SchedResource(fres))
    {
      System.out.println("== RISORSA " + codice + " ==");
      System.out.println(instance.dumpSlots(pdump));
    }
  }

  private static void findTest()
     throws Exception
  {
    System.out.println("TEST getMergerAlgos !!!");
    List<String> result = SchedMerger.getMergerAlgos();
    System.out.println("Algos: " + result + "\n");

    buildMergerResultTest("R001");
    buildMergerResultTest("R002");
    buildMergerResultTest("R003");
    buildMergerResultTest("R004");

    System.out.println("TEST find !!!");
    try(SchedMerger merger = new SchedMerger())
    {
      Properties propMerge = new Properties();
      mergeResource(merger, "R001", propMerge);
      mergeResource(merger, "R002", propMerge);
      mergeResource(merger, "R003", propMerge);
      mergeResource(merger, "R004", propMerge);

      // cerca uno o piu slot
      Properties propFind = new Properties();
      propFind.put("numSlots", "2");
      List<String> risultato = merger.findFreeSlot(propFind);

      System.out.println("RISULTATO RICERCA:");
      System.out.println(risultato);
    }

//    Properties pdump = new Properties();
//    pdump.setProperty("daystart", "0");
//    pdump.setProperty("daystop", "10");
//    dumpResource("R001", pdump);
//    dumpResource("R002", pdump);
//    dumpResource("R003", pdump);
//    dumpResource("R004", pdump);
  }
}
