from googletrans import Translator
translator = Translator();

translations = translator.translate(['じ ょ う た い / ふ つ う','ここでは　つかえません！','　は　','を　のせた！','とびさきが　みつかりません','#センターにもどります','じ ょ う た い / ふ つ う','リ ュ ッ ク','と じ る','せ て い','き せ っ し な い て た え る こ と が あ る','あ れ ? カ か か っ て い る'], dest='en')
for translation in translations:
	print(translation.origin, ' -> ', translation.text)
