#include "dataStructure.hpp"
#include "Properties.hpp"
#include "common.hpp"
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <utility>

/**
 * Inizializza uno slot file.
 */
void initSlotFile(int anno, int slotOra, int oraIniziale, int oraFinale,
                  String codiceRisorsa, SlotFile &sf) {
  if (oraIniziale >= oraFinale)
    throw StructureException("Errore definizione ore: oraIniziale deve essere "
                             "minore ma diversa da oraFinale");

  if (oraIniziale < 0 || oraIniziale > 23 || oraFinale < 0 || oraFinale > 23)
    throw StructureException("Errore definizione ore: oraIniziale e oraFinale "
                             "devono essere comprese fra 0 e 23");

  if (slotOra < 1 || slotOra >= 60)
    throw StructureException(
        "Errore definizione slotOra: deve essere compreso fra 1 e 59");

  // inizializza struttura
  memset(&sf, 0, sizeof(sf));
  memset(&sf, ' ', 2 + 16 + 4);
  strncpy(sf.magic, MAGIC, 2);
  strncpy(sf.firma, FIRMA, 16);
  snprintf(sf.sanno, 5, "%04d", anno);

  // imposta parametri nella struttura
  strncpy(sf.codiceRisorsa, codiceRisorsa.c_str(), sizeof(sf.codiceRisorsa));
  sf.anno = anno;
  sf.slotOra = slotOra;
  sf.oraIniziale = oraIniziale;
  sf.oraFinale = oraFinale;

  // calcola numeri e dimensioni
  sf.pageSize = sysconf(_SC_PAGESIZE);
  sf.numSlotsGiorno = (oraFinale - oraIniziale) * slotOra;
  sf.numSlotsTotali = sf.numSlotsGiorno * 365;
  sf.dimensioneByte = sizeof(sf) + ((sf.numSlotsTotali - 1) * sizeof(slotType));
  sf.dimensionePagine = sf.dimensioneByte / sf.pageSize;
  if ((sf.dimensioneByte % sf.pageSize) != 0)
    sf.dimensionePagine++;
  sf.dimensioneFile = sf.dimensionePagine * sf.pageSize;
}

/**
 * Inizializza uno slot file e lo salva su disco.
 */
void initSlotFile(int anno, int slotOra, int oraIniziale, int oraFinale,
                  String codiceRisorsa, SlotFile &sf, File &tosave) {
  initSlotFile(anno, slotOra, oraIniziale, oraFinale, codiceRisorsa, sf);

  int fd;
  if ((fd = open(tosave.c_str(), O_RDWR | O_CREAT | O_EXCL, 0664)) == -1) {
    if (errno == EEXIST) {
      throw StructureException(
          "Il file indicato già esiste: operazione non possibile.");
    } else {
      throw StructureException("Errore generico IO (open).");
    }
  }

  if (ftruncate(fd, sf.dimensioneFile) == -1) {
    close(fd);
    throw StructureException("Errore generico IO (ftruncate).");
  }

  lseek(fd, 0, SEEK_SET);
  write(fd, &sf, sizeof(sf));
  close(fd);
}

static String fmtfield(String des, const char *val, const char *sep) {
  return format("%-16.16s %s%s", des.c_str(), val, sep);
}

static String fmtfield(String des, int val, const char *sep) {
  return format("%-16.16s %d%s", des.c_str(), val, sep);
}

#define TOS(x) fmtfield(#x, sf.x, s)

String toString(const SlotFile &sf, String separator /*= "\n"*/) {
  const char *s = separator.c_str();

  String tmp = TOS(magic) + TOS(codiceRisorsa) + TOS(anno) + TOS(slotOra) +
               TOS(oraIniziale) + TOS(oraFinale) + TOS(numSlotsGiorno) +
               TOS(numSlotsTotali) + TOS(dimensioneByte) +
               TOS(dimensionePagine) + TOS(pageSize) + TOS(dimensioneFile);

  return tmp;
}

String dump(const SlotFile &sf, int dayStart /*= 0*/, int dayStop /*= 365*/,
            String separator /*= "\n"*/) {
  String rv, bo;
  rv.reserve((sf.numSlotsTotali * sizeof(slotType)) + 1000);
  std::map<int, String> display;
  display[0] = "_";
  display[1] = ".";
  display[2] = "B";
  display[3] = "X";

  // tabella orario; solo se c'è spazio (slotOra > 2)
  if (sf.slotOra > 2) {
    int ora = sf.oraIniziale;
    bo.append("   ");
    for (int i = 0; i < sf.numSlotsGiorno; i++) {
      if ((i % sf.slotOra) == 0)
        bo.append(format("|%-*d", sf.slotOra, ora++));
    }
    bo.append("|").append(separator);
    rv.append(bo);
  }

  // determina indirizzo del primo slot da visualizzare
  int offset = dayStart * sf.numSlotsGiorno;
  const slotType *ptSlot = sf.arrySlot + offset;

  for (int g = dayStart; g < dayStop; g++) {
    String giorno = format("%03d", g + 1);
    rv.append(giorno);
    for (int i = 0; i < sf.numSlotsGiorno; i++) {

      if ((i % sf.slotOra) == 0)
        rv.append("|");

      rv.append(display[ptSlot->status]);
      ptSlot++;
    }
    rv.append("|");
    rv.append(giorno);
    rv.append(separator);
  }

  if (sf.slotOra > 2) {
    int ora = sf.oraIniziale;
    rv.append(bo);
  }

  return rv;
}

static String strfield(const char *val) { return val; }

static String strfield(int val) { return format("%d", val); }

#define TOPROP(x) prop[#x] = strfield(sf.x)

void toProperties(const SlotFile &sf, Properties &prop) {
  TOPROP(magic);
  TOPROP(codiceRisorsa);
  TOPROP(anno);
  TOPROP(slotOra);
  TOPROP(oraIniziale);
  TOPROP(oraFinale);
  TOPROP(numSlotsGiorno);
  TOPROP(numSlotsTotali);
  TOPROP(dimensioneByte);
  TOPROP(dimensionePagine);
  TOPROP(pageSize);
  TOPROP(dimensioneFile);
}

#define TOXML(x)                                                               \
  rv.append("<header name=\"#x\">" + strfield(sf.x) + "</header>")

String toXML(const SlotFile &sf, int dayStart /*= 0*/, int dayStop /*= 365*/,
             String rootName /*= "resource"*/, String separator /*= "\n"*/) {
  String rv;
  rv.reserve(1024 + (sf.dimensioneByte * 4));

  rv.append("<" + rootName + ">").append(separator);

  rv.append("<headers>");
  rv.append(separator);
  TOXML(magic);
  TOXML(codiceRisorsa);
  TOXML(anno);
  TOXML(slotOra);
  TOXML(oraIniziale);
  TOXML(oraFinale);
  TOXML(numSlotsGiorno);
  TOXML(numSlotsTotali);
  TOXML(dimensioneByte);
  TOXML(dimensionePagine);
  TOXML(pageSize);
  TOXML(dimensioneFile);
  rv.append("</headers>").append(separator);

  rv.append("<slots>").append(separator);

  // determina indirizzo del primo slot da visualizzare
  int offset = dayStart * sf.numSlotsGiorno;
  const slotType *ptSlot = sf.arrySlot + offset;

  for (int g = dayStart; g < dayStop; g++) {
    for (int i = 0; i < sf.numSlotsGiorno; i++) {
      rv.append(
            format("<slot day=\"%d\" pos=\"%d\" state=\"%d\" unique=\"%d\" />",
                   g, i, (int)ptSlot->status, (int)ptSlot->info))
          .append(separator);
      ptSlot++;
    }
  }

  rv.append("</slots>").append(separator);
  rv.append("</" + rootName + ">").append(separator);

  return rv;
}
