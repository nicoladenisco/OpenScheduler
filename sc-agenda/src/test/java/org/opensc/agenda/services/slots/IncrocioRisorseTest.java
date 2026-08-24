package org.opensc.agenda.services.slots;

import static org.junit.jupiter.api.Assertions.assertEquals;

import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import org.junit.jupiter.api.Test;

public class IncrocioRisorseTest
{
  @Test
  public void generaIncroci_quandoNgEPiuGruppi_alloraCartesianoCompletoConNgInTesta()
  {
    Map<String, List<String>> risorse = new LinkedHashMap<>();
    risorse.put("NG", List.of("R1", "R2"));
    risorse.put("A", List.of("RA1", "RA2"));
    risorse.put("B", List.of("RB1", "RB2"));

    List<List<String>> incroci = IncrocioRisorse.generaIncroci(risorse);

    List<List<String>> attesi = List.of(
      List.of("R1", "R2", "RA1", "RB1"),
      List.of("R1", "R2", "RA1", "RB2"),
      List.of("R1", "R2", "RA2", "RB1"),
      List.of("R1", "R2", "RA2", "RB2"));

    assertEquals(attesi, incroci);
  }

  @Test
  public void generaIncroci_quandoSoloNg_alloraUnaSolaCombinazione()
  {
    Map<String, List<String>> risorse = new LinkedHashMap<>();
    risorse.put("NG", List.of("R1", "R2"));

    List<List<String>> incroci = IncrocioRisorse.generaIncroci(risorse);

    assertEquals(1, incroci.size());
    assertEquals(List.of("R1", "R2"), incroci.get(0));
  }

  @Test
  public void generaIncroci_quandoNgAssente_alloraCartesianoDeiGruppi()
  {
    Map<String, List<String>> risorse = new LinkedHashMap<>();
    risorse.put("A", List.of("RA1", "RA2"));

    List<List<String>> incroci = IncrocioRisorse.generaIncroci(risorse);

    List<List<String>> attesi = List.of(
      List.of("RA1"),
      List.of("RA2"));

    assertEquals(attesi, incroci);
  }

  @Test
  public void generaIncroci_quandoNgVuota_alloraCartesianoDeiGruppi()
  {
    Map<String, List<String>> risorse = new LinkedHashMap<>();
    risorse.put("NG", List.of());
    risorse.put("A", List.of("RA1", "RA2"));
    risorse.put("B", List.of("RB1", "RB2"));

    List<List<String>> incroci = IncrocioRisorse.generaIncroci(risorse);

    List<List<String>> attesi = List.of(
      List.of("RA1", "RB1"),
      List.of("RA1", "RB2"),
      List.of("RA2", "RB1"),
      List.of("RA2", "RB2"));

    assertEquals(attesi, incroci);
  }

  @Test
  public void generaIncroci_quandoNgAssenteESenzaGruppi_alloraListaVuota()
  {
    Map<String, List<String>> risorse = new LinkedHashMap<>();

    List<List<String>> incroci = IncrocioRisorse.generaIncroci(risorse);

    assertEquals(List.of(), incroci);
  }

  @Test
  public void generaIncroci_quandoNgVuotaESenzaGruppi_alloraListaVuota()
  {
    Map<String, List<String>> risorse = new LinkedHashMap<>();
    risorse.put("NG", List.of());

    List<List<String>> incroci = IncrocioRisorse.generaIncroci(risorse);

    assertEquals(List.of(), incroci);
  }

  @Test
  public void generaIncroci_quandoTreGruppi_alloraCardinalitaEOrdineAttesi()
  {
    Map<String, List<String>> risorse = new LinkedHashMap<>();
    risorse.put("NG", List.of("R1", "R2"));
    risorse.put("A", List.of("RA1", "RA2"));
    risorse.put("B", List.of("RB1", "RB2"));
    risorse.put("C", List.of("RC1", "RC2"));

    List<List<String>> incroci = IncrocioRisorse.generaIncroci(risorse);

    assertEquals(8, incroci.size());

    List<List<String>> attesi = List.of(
      List.of("R1", "R2", "RA1", "RB1", "RC1"),
      List.of("R1", "R2", "RA1", "RB1", "RC2"),
      List.of("R1", "R2", "RA1", "RB2", "RC1"),
      List.of("R1", "R2", "RA1", "RB2", "RC2"),
      List.of("R1", "R2", "RA2", "RB1", "RC1"),
      List.of("R1", "R2", "RA2", "RB1", "RC2"),
      List.of("R1", "R2", "RA2", "RB2", "RC1"),
      List.of("R1", "R2", "RA2", "RB2", "RC2"));

    assertEquals(attesi, incroci);
  }
}
