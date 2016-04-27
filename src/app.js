
function iconFromWeatherId(weatherId) {
  if (weatherId < 600) {
    return 2;
  } else if (weatherId < 700) {
    return 3;
  } else if (weatherId > 800) {
    return 1;
  } else {
    return 0;
  }
}


function fetchCity(latitude, longitude) {
  var city = "";
  console.log("reverse geocoding location...");
  var lreq = new XMLHttpRequest();
  lreq.open("GET", "https://maps.googleapis.com/maps/api/geocode/json?latlng=" + latitude + "," + longitude );//+ '&key=AIzaSyA1ecdYJBuJI2qzZiDXP6AcZ84z48SGa-g' );
  lreq.onload = function(e) {
    if (lreq.readyState == 4) {
      if (lreq.status == 200) {
          //console.log(lreq.responseText);
          var response = JSON.parse(lreq.responseText);
          city = response.results[1].formatted_address;
        console.log("google's response: " + city);
        fetchWeather(city);
      }
    }
    //console.log(lreq.responseText);
  };
  lreq.send(null);
  //console.log("fetchCity returns: " + city);
  return city;
}

function fetchWeather(city) {
  
  console.log("fetching weather for: " + city);
  city = encodeURIComponent(city.trim());
  console.log("converted to: " + city);
  var req = new XMLHttpRequest();
  console.log("request weather at: " + "http://api.openweathermap.org/data/2.5/weather?q=" + city + "&appid=d859ab87190fd80d883f126f043d8a69");
  req.open('GET', "http://api.openweathermap.org/data/2.5/weather?q=" + city + "&appid=d859ab87190fd80d883f126f043d8a69"); //"http://api.openweathermap.org/data/2.5/weather?id=4469160"); //"http://api.openweathermap.org/data/2.5/weather?" +
    //"lat=" + latitude + "&lon=" + longitude + "&cnt=1", true);
  
  req.onload = function(e) {
    if (req.readyState == 4) {
      if(req.status == 200) {
        console.log("openWeatherMap's response: " + req.responseText);

        var response = JSON.parse(req.responseText);
        var temperature = Math.round((response.main.temp - 273.15)*1.8 + 32);
        var icon = iconFromWeatherId(response.weather[0].id);
        var condition = response.weather[0].main;
        var cityName = response.name;
        console.log(temperature);
        console.log(icon);
        console.log(cityName);
        console.log(condition);
        Pebble.sendAppMessage({
          //"WEATHER_ICON_KEY":icon,
          "WEATHER_TEMPERATURE_KEY":temperature + " F",//"\u00B0F",
          "WEATHER_CITY_KEY":cityName,
          "WEATHER_CONDITION_KEY":condition}
        );

      } else {
        console.log("Error");
      }
    }
  };
  //lreq.send(null);
  req.send(null);
}

function locationSuccess(pos) {
  var coordinates = pos.coords;
  console.log ("latitude: " + coordinates.latitude);
  console.log ("longitude: " + coordinates.longitude);
  var theCity = fetchCity(coordinates.latitude, coordinates.longitude);
  console.log("theCity: " + theCity);
  fetchWeather(theCity);//coordinates.latitude, coordinates.longitude);
  //fetchWeather(35.5, -77.49);
}

function locationError(err) {
  console.warn('location error (' + err.code + '): ' + err.message);
  Pebble.sendAppMessage({
    "WEATHER_CITY_KEY":"Loc Unavailable",
    "WEATHER_TEMPERATURE_KEY":"N/A"
  });
}

var locationOptions = { "timeout": 15000, "maximumAge": 60000 }; 

Pebble.addEventListener("ready", function(e) {
  console.log("connect!" + e.ready);
  locationWatcher = window.navigator.geolocation.getCurrentPosition(locationSuccess, locationError, locationOptions);
  console.log(e.type);
});

Pebble.addEventListener('showConfiguration', function(e) {
  // Show config page
  Pebble.openURL('http://wilsonsgethitched.com/Pebble/Index');
});

Pebble.addEventListener("appmessage", function(e) {
  window.navigator.geolocation.getCurrentPosition(locationSuccess, locationError, locationOptions);
  console.log(e.type);
  console.log(e.payload.temperature);
  console.log("message!");
});

Pebble.addEventListener("webviewclosed", function(e) {
  console.log("webview closed");
  console.log(e.type);
  console.log(e.response);
});
