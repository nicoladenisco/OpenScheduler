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
     throws Exception
  {
    try(SchedResource rv = new SchedResource())
    {
      String pipeList = rv.getStamperAlgosNative();
      if("ERROR".equals(pipeList))
        throw new OscNativeException(rv.nativeError);
      return Utils.String2List(pipeList);
    }
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

  private native String dumpToXmlNative(String pipeProps);

  /**
   * Dump in formato XML della risorsa.
   * @param properties opzioni
   * @return XML con header e slots
   * @throws OscNativeException
   */
  public String dumpToXml(Properties properties)
     throws OscNativeException
  {
    String prop = Utils.Properties2String(properties);
    String rv = dumpToXmlNative(prop);
    if("ERROR".equals(rv))
      throw new OscNativeException(nativeError);
    return rv;
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
   * <li>lockDelayMillis - attesa per lock delle risorse (default 3000)</li>
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
   * <li>lockDelayMillis - attesa per lock delle risorse (default 3000)</li>
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
   * <br>
   * Parametri:
   * <ul>
   * <li>numSlots - numero di slot consecutivi (default 1)</li>
   * <li>force - ignora stato precedente dello slot (default false)</li>
   * <li>lockDelayMillis - attesa per lock delle risorse (default 3000)</li>
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
   * <br>
   * Parametri:
   * <ul>
   * <li>force - ignora stato precedente dello slot (default false)</li>
   * <li>lockDelayMillis - attesa per lock delle risorse (default 3000)</li>
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
