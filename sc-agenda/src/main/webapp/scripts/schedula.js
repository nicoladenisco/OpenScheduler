

function caricaRisorseBox(codPrest) {
  var url = jsContextPath + "/json/risorseview";
  var dati = {
    codPrest: codPrest,
    renderStart: "",
    renderEnd: ""
  };

  chiamaAjax("GET", url, dati, function (result) {
    var html
            = "<table width='100%'>"
            + "<thead>"
            + "<tr><th>&nbsp;</th><th>codice</th><th>descrizione</th><th>gruppo</th></tr>"
            + "</thead>"
            + "<tbody>";

    result.risorse.forEach(function (r) {
      html += `<tr><td style='width: 20px; background-color: ${r.backgroundcolor}'>&nbsp;</td><td>${r.code}</td><td>${r.name}</td><td>${formatGruppo(r.group)}</td></tr>`;
    });

    html += "</tbody></table>";
    $("#risorse-box").html(html);
  });
}

function formatGruppo(gruppo) {
  return gruppo ? gruppo : "NESSUNO";
}

function chiamaAjax(metodo, url, dati, funsuccess) {
  jQuery.ajax({
    url: url,
    method: metodo,
    dataType: "json",
    async: true,
    data: dati,
    success: funsuccess,
    error: function (jqxhr, textStatus, error) {
      var err = textStatus + ", " + error + "\n" + jqxhr.responseText;
      console.log("Request Failed in chiamaAjax: " + err);
    }
  });
}