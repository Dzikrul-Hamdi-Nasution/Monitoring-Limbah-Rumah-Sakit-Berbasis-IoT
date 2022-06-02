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
  

  
  $(document).ready(function() {
    
    amonia();
    ph();
    keruh();  
    oxygen();
  });


  
  function amonia() {
    var messagesRef = firebase.database();
    var ref = messagesRef.ref("Amonia");
        ref.on("value", function(snapshot) {
            snapshot.forEach(function(childSnapshot) {
                data = childSnapshot.val();
                console.log(data);
                $("#gas").val(data+" mg/L");
                if(data>5){
                    alert("Limbah Cair Tidak Memenuhi Syarat ( Amonia berlebih )")
                }
            });
        })
  }

  function ph() {
    var messagesRef = firebase.database();
    var ref = messagesRef.ref("Ph");
        ref.on("value", function(snapshot) {
            snapshot.forEach(function(childSnapshot) {
                data = childSnapshot.val();
                console.log(data);
                $("#ph").val(data);
                if(data>9){
                    alert("Limbah Cair Tidak Memenuhi Syarat ( Ph berlebih )")
                }
            });
        })
  }

  function keruh() {
    var messagesRef = firebase.database();
    var ref = messagesRef.ref("Keruh");
        ref.on("value", function(snapshot) {
            snapshot.forEach(function(childSnapshot) {
                data = childSnapshot.val();
                console.log(data);
                $("#keruh").val(data+" NTU");
                if(data>25){
                    alert("Limbah Cair Tidak Memenuhi Syarat ( Turbidity berlebih )")
                }
            });
        })
  }

  function oxygen() {
    var messagesRef = firebase.database();
    var ref = messagesRef.ref("Oxygen");
        ref.on("value", function(snapshot) {
            snapshot.forEach(function(childSnapshot) {
                data = childSnapshot.val();
                console.log(data);
                $("#do").val(data+" mg/L");
                if(data>20){
                    alert("Limbah Cair Tidak Memenuhi Syarat ( Oxygen Berlebih )")
                }
            });
        })
  }

  function oke() {

    firebase.auth().signOut();
    


    }

    
    
    firebase.auth().onAuthStateChanged(function(user) {
        if (user) {
            // User is signed in.
           
           ///alert("Database telah di Sinkronkan");
        } else {
            // No user is signed in.
            alert("Silahkan Login Kembali");
            window.location.replace("index.html")
        }
      });


    







  


