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
import java.lang.ref.Cleaner;
import java.util.List;
import java.util.Properties;

/**
 * Risorsa da schedulare.
 *
 * @author Nicola De Nisco
 */
public class SchedResource implements AutoCloseable
{
  public static final int SLOT_UNAVAILABLE = 0;
  public static final int SLOT_SCHEDULABLE = 1;
  public static final int SLOT_LOOKED = 2;
  public static final int SLOT_BOOKED = 3;

  private long nativeAddress;
  private String nativeError;
  private static final Cleaner cleaner = Cleaner.create();
  private Cleaner.Cleanable cleanable;

  /**
   * Apre una risorsa.
   * @param risorsa file da aprire
   * @throws OscNativeException
   */
  public SchedResource(File risorsa)
     throws OscNativeException
  {
    nativeAddress = 0;
    cleanable = cleaner.register(this, () -> closeNative());
    if(openNative(risorsa.getAbsolutePath()) != 0)
      throw new OscNativeException(nativeError);
  }

  private SchedResource()
  {
    nativeAddress = 0;
    cleanable = cleaner.register(this, () -> closeNative());
  }

  public static native void setDebugMode(int debugMode);

  /**
   * creazione oggetto c++.
   * @param path
   */
  private native int openNative(String path);

  /**
   * distruzione oggetto c++.
   */
  private native int closeNative();

  @Override
  public void close()
     throws Exception
  {
    if(closeNative() != 0)
      throw new OscNativeException(nativeError);
  }

  private native int buildNative(String pipeProps);

  /**
   * Crea una nuova risorsa.
   * @param properties parametri per la creazione
   * @return la risorsa creata
   * @throws OscNativeException
   */
  public static SchedResource build(Properties properties)
     throws OscNativeException
  {
    String prop = Utils.Properties2String(properties);
    SchedResource rv = new SchedResource();
    if(rv.buildNative(prop) != 0)
      throw new OscNativeException(rv.nativeError);
    return rv;
  }

  private native int clearAllSlotsNative(int stato);

  public void clearAllSlots(int stato)
     throws OscNativeException
  {
    if(clearAllSlotsNative(stato) != 0)
      throw new OscNativeException(nativeError);
  }

  private native int stampResourcesNative(String algo, String prop);

  /**
   * Imposta slots iniziali in una risorsa.
   * ATTENZIONE: cancella gli slot oggetto dello stamper.
   * @param algoName nome dell'algoritmo
   * @param properties parametri operazione (dipende dall'algoritmo)
   * @throws OscNativeException
   */
  public void stampResources(String algoName, Properties properties)
     throws OscNativeException
  {
    String prop = Utils.Properties2String(properties);
    stampResourcesNative(algoName, prop);
  }

  private native String getStamperAlgosNative();

  /**
   * Ritorna un elenco degli algoritmi stamper implementati.
   * @return lista degli algoritmi
   * @throws OscNativeException
   */
  public static List<String> getStamperAlgos()
     throws OscNativeException
  {
    SchedResource rv = new SchedResource();
    String pipeList = rv.getStamperAlgosNative();
    if("ERROR".equals(pipeList))
      throw new OscNativeException(rv.nativeError);
    return Utils.String2List(pipeList);
  }

  private native String dumpHeaderNative(String pipeProps);

  public String dumpHeader(Properties properties)
     throws OscNativeException
  {
    String prop = Utils.Properties2String(properties);
    String rv = dumpHeaderNative(prop);
    if("ERROR".equals(rv))
      throw new OscNativeException(nativeError);
    return rv;
  }

  private native String dumpSlotsNative(String pipeProps);

  public String dumpSlots(Properties properties)
     throws OscNativeException
  {
    String prop = Utils.Properties2String(properties);
    String rv = dumpSlotsNative(prop);
    if("ERROR".equals(rv))
      throw new OscNativeException(nativeError);
    return rv;
  }
}
