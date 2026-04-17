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
import java.io.IOException;
import java.lang.ref.Cleaner;
import java.util.Properties;

/**
 * Inizializzatore delle risorse.
 *
 * @author Nicola De Nisco
 */
public class SchedStamper implements AutoCloseable
{
  private long nativeAddress;
  private static final Cleaner cleaner = Cleaner.create();
  private Cleaner.Cleanable cleanable;

  /**
   * Apre una risorsa.
   * @param risorsa file da aprire
   * @throws IOException
   */
  public SchedStamper(File risorsa)
     throws IOException
  {
    cleanable = cleaner.register(this, () -> closeNative());
    openNative(risorsa.getAbsolutePath());
  }

  private SchedStamper()
  {
    cleanable = cleaner.register(this, () -> closeNative());
  }

  /**
   * creazione oggetto c++.
   * @param path
   */
  private native void openNative(String path);

  /**
   * distruzione oggetto c++.
   */
  private native void closeNative();

  /**
   * distruzione oggetto c++.
   */
  private native void buildNative(String prop);

  @Override
  public void close()
     throws Exception
  {
    closeNative();
  }

  public static SchedStamper build(Properties properties)
  {
    String prop = Utils.Properties2String(properties);
    SchedStamper rv = new SchedStamper();
    rv.buildNative(prop);
    return rv;
  }
}
