
#include <map>
#include <string>
#include <vector>

template <typename K, typename T>
class Classificatore : private std::map<K, std::vector<T>> {
  // Alias interno per comodità
  using Base = std::map<K, std::vector<T>>;

public:
  Classificatore() = default;

  template <typename Func>
  Classificatore(const std::vector<T> &elementi, Func estraiChiave) {
    aggiungi(elementi, estraiChiave);
  }

  // Riportiamo alla luce solo i metodi che ci servono
  using Base::begin;
  using Base::empty;
  using Base::end;
  using Base::find;
  using Base::size;

  // Metodo per classificare un vettore di elementi
  // EstraiChiave è un tipo generico per accettare lambda
  template <typename Func>
  void aggiungi(const std::vector<T> &elementi, Func estraiChiave) {
    for (const auto &item : elementi) {
      // Applichiamo la lambda per ottenere la chiave
      K chiave = estraiChiave(item);
      avviungi(chiave, item);
    }
  }

  // Aggiunge un metodo sicuro per inserire i dati
  void aggiungi(const K &chiave, const T &valore) {
    (*this)[chiave].push_back(valore);
  }

  // Metodo per visualizzare i risultati (opzionale, per test)
  std::string toString(std::string separator = ",") const {
    std::string rv;
    rv.reserve(1024);

    for (const auto &[chiave, lista] : *this) {

      if (!rv.empty())
        rv.append(separator);

      rv.append("[")
          .append(chiave)
          .append("{")
          .append(lista.size())
          .append("}]");
    }

    return rv;
  }
};
