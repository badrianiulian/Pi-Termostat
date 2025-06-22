function TogglePass(id) {
    var passElem = document.getElementById(id);
    if (((typeof getComputedStyle !== 'undefined')?getComputedStyle(passElem):passElem.currentStyle).webkitTextSecurity) {
      // webkit present so do nothing
    } else {
      passElem.setAttribute("type", "password");
    }
}

function encrypt(str,KEY,IV) {
    var key = CryptoJS.enc.Utf8.parse(KEY); // Secret key
    var iv  =  CryptoJS.enc.Utf8.parse(IV); // Vector iv
    var encrypted = CryptoJS.AES.encrypt(str, key, { iv: iv, mode: CryptoJS.mode.CBC, padding: CryptoJS.pad.Pkcs7});
    return encrypted.toString();
}

function decrypt(str,KEY,IV) {
    var key = CryptoJS.enc.Utf8.parse(KEY); // Secret key
    var iv  =  CryptoJS.enc.Utf8.parse(IV); // Vector iv
    var decrypted = CryptoJS.AES.decrypt(str, key, { iv: iv, padding: CryptoJS.pad.Pkcs7});
    return decrypted.toString(CryptoJS.enc.Utf8);
}

function usernameIsValid(username) {
    return /^[0-9a-zA-Z_.-]+$/.test(username);
}
