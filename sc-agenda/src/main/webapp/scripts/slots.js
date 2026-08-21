

function caricaSlotBox(codPrest, start, end) {
  var url = jsContextPath + "/json/slot";
  var dati = {
    codPrest: codPrest,
    renderStart: start,
    renderEnd: end
  };

  chiamaAjaxAsync("GET", url, dati, function (result) {
    var html = "";
    result.risorse.forEach(function (r) {
      html += `<div class='slot-box-fixed'>Slots ${r.code} (${r.name})\n${r.sldump}</div>`;
    });
    $("#slot-risorse-row").html(html);

    html = "";
    result.incroci.forEach(function (r) {
      html += `<div class='slot-box-fixed'>Slots ${r.name}\n${r.sldump}</div>`;
    });
    $("#slot-incroci-row").html(html);
  });
}

function formatGruppo(gruppo) {
  return gruppo ? gruppo : "NESSUNO";
}
