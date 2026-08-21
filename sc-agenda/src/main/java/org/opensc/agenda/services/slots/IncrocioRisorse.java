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

import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

/**
 * Genera gli incroci fra risorse raggruppate in una mappa.
 * <p>
 * La chiave 'NG' individua le risorse non raggruppate: ognuna di esse genera
 * un proprio set di incroci. Le altre chiavi (gruppi di equivalenza, es. 'A',
 * 'B', ...) contengono risorse fra loro equivalenti: per ogni risorsa NG viene
 * generata una combinazione "baseline" (il primo elemento di ciascun gruppo),
 * seguita da una variante per ciascun elemento successivo di ciascun gruppo,
 * tenendo fissi gli altri gruppi alla baseline (schema "one-factor-at-a-time").
 *
 * @author Nicola De Nisco
 */
public class IncrocioRisorse
{
  public static final String CHIAVE_NON_RAGGRUPPATE = "NG";

  /**
   * Genera gli incroci di risorse secondo la regola descritta nella classe.
   *
   * @param <T> tipo della risorsa
   * @param risorse mappa delle risorse raggruppate per chiave
   * @return lista degli incroci; ogni incrocio è una lista di risorse, una per
   * ciascun gruppo (NG compreso, in prima posizione)
   */
  public static <T> List<List<T>> generaIncroci(Map<String, List<T>> risorse)
  {
    List<T> nonRaggruppate = risorse.get(CHIAVE_NON_RAGGRUPPATE);
    if(nonRaggruppate == null || nonRaggruppate.isEmpty())
      return new ArrayList<>();

    // gruppi di equivalenza (tutte le chiavi tranne NG), nell'ordine di iterazione
    // della mappa
    Map<String, List<T>> gruppi = new LinkedHashMap<>(risorse);
    gruppi.remove(CHIAVE_NON_RAGGRUPPATE);
    List<List<T>> valoriGruppi = new ArrayList<>(gruppi.values());

    List<List<T>> risultato = new ArrayList<>();

    for(T ng : nonRaggruppate)
    {
      // baseline: risorsa NG + primo elemento di ciascun gruppo
      List<T> baseline = new ArrayList<>();
      baseline.add(ng);
      for(List<T> gruppo : valoriGruppi)
        baseline.add(gruppo.get(0));

      risultato.add(baseline);

      // varianti: un solo gruppo alla volta si discosta dalla baseline
      for(int i = 0; i < valoriGruppi.size(); i++)
      {
        List<T> gruppo = valoriGruppi.get(i);
        for(int j = 1; j < gruppo.size(); j++)
        {
          List<T> variante = new ArrayList<>(baseline);
          variante.set(i + 1, gruppo.get(j));
          risultato.add(variante);
        }
      }
    }

    return risultato;
  }
}
