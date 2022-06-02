var config = {
    apiKey: "AIzaSyDmcfLdDAAA8Z9H8F17Pq7Gh2ILcX5f4kQ",
  authDomain: "main-station-logger.firebaseapp.com",
  databaseURL: "https://main-station-logger-default-rtdb.firebaseio.com",
  projectId: "main-station-logger",
  storageBucket: "main-station-logger.appspot.com",
  messagingSenderId: "1052397997259",
  appId: "1:1052397997259:web:6b013a27bc0c5188461f4e",
  measurementId: "G-CT2TQS7PX8"
  };


firebase.initializeApp(config);
firebase.auth().onAuthStateChanged(function(user) {
  if (user) {
      // User is signed in.
     window.location.replace("Station_data.html")
     
  } else {
      // No user is signed in.
      
  }
});

function masuk() {
  var userEmail = document.getElementById("email_field").value;
  var userPass = document.getElementById("password_field").value;

  firebase.auth().signInWithEmailAndPassword(userEmail, userPass).catch(function(error) {
      // Handle Errors here.
      var errorCode = error.code;
      var errorMessage = error.message;
      window.alert("Error : " + errorMessage);
  });
}

function keluar() {
  firebase.auth().signOut();
}






  


