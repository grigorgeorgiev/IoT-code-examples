ESP32-CAMToPHPServer01
it is important to know, if you are using website with .htaccess redurects, you have to make exception as:

#stop util folder from redirects 301 - it cause problems for post requests RewriteCond %{REQUEST_URI} !^/util/

if the folder to upload is "util"
