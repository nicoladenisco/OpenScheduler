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
  public static List<String> getMergerAlgos()
     throws Exception
  {
    try(SchedMerger rv = new SchedMerger())
    {
      String pipeList = rv.getMergerAlgosNative();
      if("ERROR".equals(pipeList))
        throw new OscNativeException(rv.nativeError);
      return Utils.String2List(pipeList);
    }
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
   * Effettua l'impegno definitivo dello slot per le risorse contenute.
   * <br>
   * Parametri per la ricerca:
   * <ul>
   * <li>numSlots - numero di slot consecutivi (default 1)</li>
   * <li>force - ignora stato precedente dello slot (default false)</li>
   * <li>lockDelayMillis - attesa per lock delle risorse (default 5000)</li>
   * </ul>
   * Per orario si intende l'indice dello slot all'interno del giorno.
   * Sia i giorni che gli orari sono 0 based.
   *
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

  private native String getInfoHeaderNative();

  /**
   * Recupera informazioni sull'header.
   * @return informazioni header fusione
   */
  public Properties getInfoHeader()
  {
    String pipeMap = getInfoHeaderNative();
    return Utils.String2Properties(pipeMap);
  }

  private native String findFreeSlotNative(String pipeProps);

  /**
   * Cerca slot liberi.
   * <br>
   * Parametri per la ricerca:
   * <ul>
   * <li>daystart - giorno iniziale (default 0)</li>
   * <li>daystop - giorno finale (default 365)</li>
   * <li>numSlots - numero di slot consecutivi (default 1)</li>
   * <li>slotgiornoInizio - orario iniziale (default 0)</li>
   * <li>slotgiornoFine - orario finale (default ultimo)</li>
   * <li>lockDelayMillis - attesa per lock delle risorse (default 5000)</li>
   * </ul>
   * Per orario si intende l'indice dello slot all'interno del giorno.<br>
   * Sia i giorni che gli orari sono 0 based.
   *
   * @param properties parametri ricerca slots
   * @return lista di coppie giorno,orario possibili
   * @throws OscNativeException
   */
  public List<String> findFreeSlot(Properties properties)
     throws OscNativeException
  {
    String prop = Utils.Properties2String(properties);
    String results = findFreeSlotNative(prop);
    if("ERROR".equals(results))
      throw new OscNativeException(nativeError);
    return Utils.String2List(results);
  }

  private native int clearSlotNative(int giorno, int slotgiorno, long uniqueID, String pipeProps);

  /**
   * Libera uno o piu slot precedentemente occupato.
   * Rende liberi gli slot per tutte le risorse contenute.
   * <br>
   * Parametri:
   * <ul>
   * <li>numSlots - numero di slot consecutivi (default 1)</li>
   * <li>force - ignora stato precedente dello slot (default false)</li>
   * <li>lockDelayMillis - attesa per lock delle risorse (default 5000)</li>
   * </ul>
   * Per orario si intende l'indice dello slot all'interno del giorno.
   * Sia i giorni che gli orari sono 0 based.
   *
   * @param giorno indice del giorno (0 based)
   * @param slotgiorno numero dello slot all'interno del giorno (0 based)
   * @param uniqueID identificatore univoco per lo slot (0=ignorato)
   * @param properties opzioni di prenotazione
   * @throws OscNativeException
   */
  public void clearSlots(int giorno, int slotgiorno, long uniqueID, Properties properties)
     throws OscNativeException
  {
    String prop = Utils.Properties2String(properties);
    if(clearSlotNative(giorno, slotgiorno, uniqueID, prop) != 0)
      throw new OscNativeException(nativeError);
  }

  /**
   * Libera uno o piu slot precedentemente occupato.
   * Rende liberi gli slot per tutte le risorse contenute.
   * <br>
   * Parametri:
   * <ul>
   * <li>force - ignora stato precedente dello slot (default false)</li>
   * <li>lockDelayMillis - attesa per lock delle risorse (default 5000)</li>
   * </ul>
   *
   * @param uniqueID identificatore univoco per lo slot
   * @param properties opzioni di prenotazione
   * @throws OscNativeException
   */
  public void clearSlots(long uniqueID, Properties properties)
     throws OscNativeException
  {
    String prop = Utils.Properties2String(properties);
    if(clearSlotNative(-1, -1, uniqueID, prop) != 0)
      throw new OscNativeException(nativeError);
  }
}
