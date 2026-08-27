var datiSlots;

function caricaSlotBox(codPrest, start, end) {
  var url = jsContextPath + "/json/slot";
  var dati = {
    codPrest: codPrest,
    renderStart: start,
    renderEnd: end
  };

  // pulisce dati
  datiSlots = null;

  chiamaAjaxAsync("GET", url, dati, function (result) {
    datiSlots = result;
    var html = "";
    result.risorse.forEach(function (r) {
      html += `<div class='slot-box-fixed'>Slots ${r.code} (${r.name})\n${r.sldump}</div>`;
    });
    $("#slot-risorse-row").html(html);

    html = "";
    var optionsHtml = "<option value=''>Seleziona una combinazione di risorse ...</option>";
    result.incroci.forEach(function (r) {
      html += `<div class='slot-box-fixed'>Slots ${r.descr}\n${r.sldump}</div>`;
      optionsHtml += `<option value='${r.name}'>${r.descr}</option>`;
    });
    $("#slot-incroci-row").html(html);
    $("#comboIncrocio").html(optionsHtml);
  });
}

function formatGruppo(gruppo) {
  return gruppo ? gruppo : "NESSUNO";
}

function caricaSelectDinamica(selectSelector, url, dati, valueField, textField, placeholder) {
  var $select = $(selectSelector);
  if ($select.length === 0) {
    return;
  }

  chiamaAjaxAsync("GET", url, dati || {}, function (result) {
    var items = Array.isArray(result) ? result : (result && Array.isArray(result.items) ? result.items : []);
    var optionsHtml = "";

    if (placeholder) {
      optionsHtml += `<option value=''>${placeholder}</option>`;
    }

    items.forEach(function (item) {
      var value = item[valueField];
      var text = item[textField];

      if (value !== undefined && value !== null) {
        optionsHtml += `<option value='${value}'>${text || ""}</option>`;
      }
    });

    $select.html(optionsHtml);
    $select.trigger("change");
  });
}

function caricaDisponibilitaJson(codPrest, start, end) {
  var url = jsContextPath + "/json/slot-dispo";
  var incrocio = $("#comboIncrocio").val();
  if (incrocio == "") {
    $("#tabella-dispo").html("");
    return;
  }

  var dati = {
    codPrest: codPrest,
    incrocio: incrocio,
    renderStart: start,
    renderEnd: end
  };

  chiamaAjaxAsync("GET", url, dati, function (result) {
    if (result.dispo.length === 0) {
      $("#tabella-dispo").html("Nessuna disponibilità trovata per la combinazione richiesta.");
      return;
    }

    var html = "<table width='100%'>";
    result.dispo.forEach(function (d) {
      html += `<tr>
<td>${d.gsn}</td><td>${d.date}</td><td>${d.orario}</td><td>${d.giorno}</td><td>${d.slot}</td><td>${d.gs}</td>
<td></td>
<td><button onclick=\"salvaPrenotazione('${codPrest}','${start}','${end}','${incrocio}','${d.giorno}','${d.slot}')\">Prenota</button></td>
</tr>`;
    });
    html += "</table>";
    $("#tabella-dispo").html(html);
  });
}

function caricaDisponibilita(codPrest, start, end) {
  var incrocio = $("#comboIncrocio").val();
  if (incrocio == "") {
    $("#tabella-dispo").html("");
    return;
  }

  $("#tabella-dispo").html("Nessuna disponibilità per la combinazione richiesta.");

  datiSlots.incroci.forEach(function (r) {
    if (r.name == incrocio) {
      if (r.dispo.length > 0) {
        var html = "<table width='100%'>";
        r.dispo.forEach(function (d) {
          html += `<tr>
<td>${d.gsn}</td><td>${d.date}</td><td>${d.orario}</td><td>${d.giorno}</td><td>${d.slot}</td><td>${d.gs}</td>
<td></td>
<td><button onclick=\"salvaPrenotazione('${codPrest}','${start}','${end}','${incrocio}','${d.giorno}','${d.slot}')\">Prenota</button></td>
</tr>`;
        });
        html += "</table>";
        $("#tabella-dispo").html(html);
      }
    }
  });
}

function salvaPrenotazione(codPrest, start, end, incrocio, giorno, slot) {
  var url = jsContextPath + "/json/slot-dispo";

  var dati = {
    codPrest: codPrest,
    incrocio: incrocio,
    renderStart: start,
    renderEnd: end,
    giorno: giorno,
    slot: slot
  };

  chiamaAjaxAsync("POST", url, dati, function (result) {
  });
}
