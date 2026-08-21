/* eslint-disable no-var,prefer-template,no-undef */

var sel = function (selector) {
  return document.querySelector(selector);
};

var sela = function (selector) {
  return Array.prototype.slice.call(document.querySelectorAll(selector));
};

function getNavbarRange(tzStart, tzEnd, viewType) {
  var start = tzStart.toDate();
  var end = tzEnd.toDate();
  var middle;
  if (viewType === 'month') {
    middle = new Date(start.getTime() + (end.getTime() - start.getTime()) / 2);

    return moment(middle).format('YYYY-MM');
  }
  if (viewType === 'day') {
    return moment(start).format('YYYY-MM-DD');
  }
  if (viewType === 'week') {
    return moment(start).format('YYYY-MM-DD') + ' ~ ' + moment(end).format('YYYY-MM-DD');
  }
  throw new Error('no view type');
}

function chiamaAjaxSync(metodo, url, dati, funsuccess) {
  jQuery.ajax({
    url: url,
    method: metodo,
    dataType: "json",
    async: false,
    data: dati,
    success: funsuccess,
    error: function (jqxhr, textStatus, error) {
      var err = textStatus + ", " + error + "\n" + jqxhr.responseText;
      console.log("Request Failed in chiamaAjax: " + err);
    }
  });
}

function chiamaAjaxAsync(metodo, url, dati, funsuccess) {
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

