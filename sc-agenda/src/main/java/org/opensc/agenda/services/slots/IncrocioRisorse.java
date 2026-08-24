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
 * La chiave 'NG' individua le risorse non raggruppate: se presente e non vuota,
 * tutte le risorse NG compaiono in ogni incrocio. Le altre chiavi (gruppi di
 * equivalenza, es. 'A', 'B', ...) contengono risorse fra loro equivalenti:
 * ogni incrocio seleziona esattamente una risorsa per ciascun gruppo di
 * equivalenza, generando il prodotto cartesiano completo tra i gruppi.
 *
 * @author Nicola De Nisco
 */
public class IncrocioRisorse
{
  public static final String CHIAVE_NON_RAGGRUPPATE = "NG";

  /**
   * Genera gli incroci di risorse secondo la regola descritta nella classe.
   *
   * @param <T>     tipo della risorsa
   * @param risorse mappa delle risorse raggruppate per chiave
   * @return lista degli incroci; ogni incrocio contiene (se presenti) tutte le
   *         risorse NG, seguite da una risorsa per ciascun gruppo di equivalenza
   */
  public static <T> List<List<T>> generaIncroci(Map<String, List<T>> risorse)
  {
    List<T> nonRaggruppate = risorse.get(CHIAVE_NON_RAGGRUPPATE);

    // gruppi di equivalenza (tutte le chiavi tranne NG), nell'ordine di iterazione
    // della mappa
    Map<String, List<T>> gruppi = new LinkedHashMap<>(risorse);
    gruppi.remove(CHIAVE_NON_RAGGRUPPATE);
    List<List<T>> valoriGruppi = new ArrayList<>(gruppi.values());

    List<List<T>> risultato = new ArrayList<>();

    for (List<T> gruppo : valoriGruppi)
      if (gruppo == null || gruppo.isEmpty())
        return risultato;

    List<T> prefisso = nonRaggruppate == null ? new ArrayList<>() : new ArrayList<>(nonRaggruppate);

    if (valoriGruppi.isEmpty())
    {
      if (!prefisso.isEmpty())
        risultato.add(prefisso);
      return risultato;
    }

    generaProdottoCartesiano(valoriGruppi, 0, prefisso, risultato);

    return risultato;
  }

  private static <T> void generaProdottoCartesiano(
    List<List<T>> gruppi,
    int indiceGruppo,
    List<T> corrente,
    List<List<T>> risultato)
  {
    if (indiceGruppo >= gruppi.size())
    {
      risultato.add(new ArrayList<>(corrente));
      return;
    }

    for (T risorsa : gruppi.get(indiceGruppo))
    {
      corrente.add(risorsa);
      generaProdottoCartesiano(gruppi, indiceGruppo + 1, corrente, risultato);
      corrente.remove(corrente.size() - 1);
    }
  }
}
