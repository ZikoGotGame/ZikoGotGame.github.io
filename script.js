var tablinks = document.getElementsByClassName("tab-links");
    var tabcontents = document.getElementsByClassName("tab-contents");

    function opentab(tabname){
      for(tablink of tablinks){
        tablink.classList.remove("active-link");
      }
      for(tabcontent of tabcontents){
        tabcontent.classList.remove("active-tab");
      }
      event.currentTarget.classList.add("active-link");
      document.getElementById(tabname).classList.add("active-tab");
    }

var sidemeu = document.getElementById("sidemenu");

function openmenu(){
  sidemeu.style.right = "0";
}

function closemenu(){
  sidemeu.style.right = "-200px";
}

const scriptURL = 'https://script.google.com/macros/s/AKfycbzUA6vDaPogxJVhL6NnBr2g2kl_q2Er8tsak9wSG6HWHNFR3TBrrKTmwGHoqUf2GcEh/exec'
const msg = document.getElementById("msg")

var form = document.getElementById('sheetdb-form');
form.addEventListener("submit", e => {
  e.preventDefault();
  fetch(form.action, {
      method : "POST",
      body: new FormData(document.getElementById("sheetdb-form")),
  }).then(
      response => response.json()
  ).then((html) => {
    msg.innerHTML = "Your message has been sent"
    setTimeout(function(){
      msg.innerHTML = ""
    },5000)
    form.reset()
  });
});