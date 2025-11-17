var tablinks = document.getElementsByClassName("tab-links");
var tabcontents = document.getElementsByClassName("tab-contents");

function opentab(tabname) {
  for (tablink of tablinks) {
    tablink.classList.remove("active-link");
  }
  for (tabcontent of tabcontents) {
    tabcontent.classList.remove("active-tab");
  }
  event.currentTarget.classList.add("active-link");
  document.getElementById(tabname).classList.add("active-tab");
}

var sidemeu = document.getElementById("sidemenu");

function openmenu() {
  sidemeu.style.right = "0";
}

function closemenu() {
  sidemeu.style.right = "-200px";
}

emailjs.init("Yqb1MjoCegfkNUzRP");

document
  .getElementById("sheetdb-form")
  .addEventListener("submit", function (event) {
    event.preventDefault();

    const form = event.target;

    emailjs
      .send("service_omfag1g", "template_fu7bwsx", {
        from_name: form.elements["from_name"].value,
        reply_to: form.elements["reply_to"].value,
        message: form.elements["message"].value,
      })
      .then(() => {
        msg.innerText = "Your message has been sent!";
        setTimeout(() => (msg.innerText = ""), 5000);
        form.reset();
      })
      .catch((error) => {
        msg.innerText = "Failed to send message.";
      });
  });
