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

import java.io.IOException;
import java.lang.ref.Cleaner;
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

  /**
   * Apre una risorsa.
   * @param risorsa file da aprire
   * @throws IOException
   */
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
}
