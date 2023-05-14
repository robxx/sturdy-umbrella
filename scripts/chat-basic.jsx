#include json2.js

apikey = ""
auth = "Authorization: Bearer " + apikey

model = "gpt-3.5-turbo"
sys = "You rephrase a paragraph of text into clear and concise English."
z = "The Labour Party is a democratic socialist party. It believes that by the strength of our common endeavour we achieve more than we achieve alone, so as to create for each of us the means to realise our true potential and for all of us a community in which power, wealth and opportunity are in the hands of the many, not the few, where the rights we enjoy reflect the duties we owe, and where we live together, freely, in a spirit of solidarity, tolerance and respect."
messages=[
        {"role": "system", "content": sys },
        {"role": "user", "content": z },
    ]   
    
data = {
 "model": model,
 "messages": messages,
 "temperature": 0.7
}
//$.write(JSON.stringify(data))

libFilename = "ESCurl.dll" 
Folder.current = File ($.fileName).parent
libPath = Folder.current.fsName + "\\" + libFilename;
$.write(libPath)
curl = new ExternalObject("lib:" + libPath);

receivedText = curl.httpPost("https://api.openai.com/v1/chat/completions", JSON.stringify(data), auth);

x = JSON.parse(receivedText);
y = x.choices[0].message.content

$.write(y + "\n\n\n\n");

curl.unload();