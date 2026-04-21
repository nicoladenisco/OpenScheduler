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

import java.lang.ref.Cleaner;
import java.util.List;
import java.util.Properties;

/**
 * Fusore di risorse.
 *
 * @author Nicola De Nisco
 */
public class SchedMerger implements AutoCloseable
{
  private long nativeAddress;
  private String nativeError;
  private static final Cleaner cleaner = Cleaner.create();
  private Cleaner.Cleanable cleanable;
  private long unique;

  public SchedMerger()
     throws OscNativeException
  {
    nativeAddress = 0;
    unique = System.currentTimeMillis();
    cleanable = cleaner.register(this, () -> closeNative());
    if(openNative(unique) != 0)
      throw new OscNativeException(nativeError);
  }

  /**
   * creazione oggetto c++.
   * @param path
   */
  private native int openNative(long unique);

  /**
   * distruzione oggetto c++.
   */
  private native int closeNative();

  @Override
  public void close()
     throws Exception
  {
    closeNative();
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

  private native String getMergerAlgosNative();

  /**
   * Ritorna un elenco degli algoritmi merger implementati.
   * @return lista degli algoritmi
   * @throws OscNativeException
   */
  public List<String> getMergerAlgos()
     throws OscNativeException
  {
    String pipeList = getMergerAlgosNative();
    if("ERROR".equals(pipeList))
      throw new OscNativeException(nativeError);
    return Utils.String2List(pipeList);
  }

  private native int mergeResourcesNative(String algoName, String pipeProps);

  /**
   * Aggiunge una risorsa al merger.
   * @param algoName nome dell'algoritmo
   * @param properties parametri operazione (dipende dall'algoritmo)
   * @throws OscNativeException
   */
  public void mergeResources(String algoName, Properties properties)
     throws OscNativeException
  {
    String prop = Utils.Properties2String(properties);
    if(mergeResourcesNative(algoName, prop) != 0)
      throw new OscNativeException(nativeError);
  }

  private native String getResourcesListNative();

  /**
   * Ritorna un elenco delle risorse presenti nel merger.
   * @return lista codici risorsa
   * @throws OscNativeException
   */
  public List<String> getResourcesList()
     throws OscNativeException
  {
    String pipeList = getResourcesListNative();
    if("ERROR".equals(pipeList))
      throw new OscNativeException(nativeError);
    return Utils.String2List(pipeList);
  }

  private native int clearResourcesNative();

  /**
   * Pulisce il merger rilasciando tutte le risorse.
   * Gli slot occupati temporaneamente vengono rilasciati.
   * @throws OscNativeException
   */
  public void clearResources()
     throws OscNativeException
  {
    if(clearResourcesNative() != 0)
      throw new OscNativeException(nativeError);
  }

  private native int reserveSlotNative(int giorno, int slotgiorno, long uniqueID, String pipeProps);

  /**
   * Riserva uno slot fra quelli liberi del merger.
   * Effettua l'impegno definitivo dello slot per la risorsa indicata.
   * @param giorno indice del giorno (0 based)
   * @param slotgiorno numero dello slot all'interno del giorno (0 based)
   * @param uniqueID identificatore univoco per lo slot
   * @param properties opzioni di prenotazione
   * @throws OscNativeException
   */
  public void reserveSlot(int giorno, int slotgiorno, long uniqueID, Properties properties)
     throws OscNativeException
  {
    String prop = Utils.Properties2String(properties);
    if(reserveSlotNative(giorno, slotgiorno, uniqueID, prop) != 0)
      throw new OscNativeException(nativeError);
  }
}
