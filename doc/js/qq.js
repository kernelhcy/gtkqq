var sys={
	$:function(A){
		return document.getElementById(A)
	},
	onload:function(B,C){
		var A=window.onload;window.onload=function(){
			if(C){
				if(typeof (B)=="function"){
					B()
				}
				if(typeof (A)=="function"){
					A()
				}
			}else{
				if(typeof (A)=="function"){
					A()
				}
				if(typeof (B)=="function"){
					B()
				}
			}
		}
	}
};
var pt={
	isHttps:false,err_m:null,
	init:function(){
		try{
			pt.isHttps=((window.parent.location+"")
				.substring(0,5)
				.toLowerCase()=="https")
		}catch(A){}
		sys.onload(
			function(){
				pt.err_m=sys.$("err_m")
			}
		)
	},
	show_err:function(A){
		if(pt.err_m&&(typeof ptui_notifySize=="function")){
			pt.err_m.innerHTML=A;
			pt.err_m.style.display="block";
			ptui_notifySize("login");
			return 
		}else{
			alert(A)
		}
	}
};
pt.init();
var vc_type="";
var lastUin=1;
var t_appid=46000101;
var g_changeNum=0;
var g_checkTime=0;
var g_imgTime=0;
var first=true;
var changeimg=false;
var defaultuin="";
var login_param=g_href.substring(g_href.indexOf("?")+1);
function ptui_onEnableLLogin(B){
	var A=B.low_login_enable;
	var C=B.low_login_hour;
	if(A!=null&&C!=null){
		C.disabled=!A.checked
	}
}
function ptui_setDefUin(C,B){
	if(B==""||B==null){
		B=getCookie("ptui_loginuin");
		if(B){
			defaultuin=B
		}
	}
	if(g_appid==t_appid){
		var A=getCookie("ptui_loginuin2");
		if(A){
			B=A;defaultuin=B
		}
	}
	if(B!=""&&B!=null){
		C.u.value=B
	}
}

var g_ptredirect=-1;
var g_xmlhttp;
var g_loadcheck=true;
var g_submitting=false;

function ptui_needVC(C,D){
	if(t_appid==D){
		if((C.indexOf("@")<0)&&isNaN(C)){
			C="@"+C
		}
	}
	var B="";
	if(pt.isHttps){
		ptui_checkVC("1","");
		return 
	}else{
		B="http://ptlogin2."+g_domain+"/check?uin="
			+C+"&appid="+D+"&r="+Math.random()
	}
	var A=document.createElement("script");
	g_imgTime=new Date();
	A.src=B;document.body.appendChild(A);
	g_loadcheck=true;
	return 
}
function ptui_checkVC(A,B){
	g_loadcheck=false;
	g_checkTime=new Date().getTime()-g_checkTime;
	if(g_submitting){
		return 
	}
	var D=new Date();
	if(defaultuin!=""&&g_changeNum<=1){
		g_time.time7=D;
		var C={"12":g_time.time7-g_time.time6};
		if(defaultuin!=""){
			C["16"]=g_time.time6-g_time.time3,
			C["17"]=g_time.time7-g_time.time3
		}
		if(!xuiFrame){
			ptui_speedReport(C)
		}
	}else{
		g_time.time10=D;
		var C={"13":g_time.time10-g_time.time9};
		ptui_speedReport(C)
	}
	if(pt.isHttps){
		A=1
	}
	if(A=="0"){
		$("verifycode").value=B;
		loadVC(false)
	}else{
		vc_type=B;
		$("verifycode").value="";
		loadVC(true)
	}
}

function ptui_trim(A){
	return A.replace(/(^\s*)|(\s*$)/g,"")
}

function ptui_checkQQUin(qquin){
	if(qquin.length==0){
		return false
	}
	qquin=ptui_trim(qquin);
	if((new RegExp(/^[a-zA-Z]{1}([a-zA-Z0-9]|[-_]){0,19}$/).test(qquin))){
		if(t_appid==g_appid){
			return true
		}else{
			$("u").value=ptui_trim($("u").value)+"@qq.com";
			qquin=$("u").value
		}
	}if(!(new RegExp(/^\w+((-\w+)|(\.\w+))*\@[A-Za-z0-9]+((\.|-)[A-Za-z0-9]+)*\.[A-Za-z0-9]+$/).test(qquin))){
		if(qquin.length<5||qquin.length>12
				||parseInt(qquin)<1000){
			return false
		}
		var exp=eval("/^[0-9]*$/");
		return exp.test(qquin)
	}
	return true
}

function ptui_changeImgEx(D,C,G,F){
	changeimg=true;
	var A=$("imgVerify");
	if(!pt.isHttps){
		F+=("&vc_type="+vc_type)
	}
	try{
		if(A!=null){
			A.src=F+"&aid="+C+"&"+Math.random();
			var B=$("verifycode");
			if(B!=null&&B.disabled==false&&G){
				B.focus();
				B.select()
			}
		
	}}catch(E){}
}

function ptui_changeImg(B,A,D){
	var C=g_uin;
	if((g_appid==t_appid)&&isNaN(g_uin)&&(g_uin.indexOf("@")<0)){
		C="@"+g_uin
	}
	ptui_changeImgEx(B,A,D,"http://captcha."+B+"/getimage?&uin="+C)
}

function ptui_changeImgHttps(B,A,D){
	var C=g_uin;
	if((g_appid==t_appid)&&isNaN(g_uin)&&(g_uin.indexOf("@")<0)){
		C="@"+g_uin
	}
	ptui_changeImgEx(B,A,D,"./getimage?uin="+C)
}

function ptui_initFocus(B){
	try{
		var A=B.u;
		var D=B.p;
		var E=B.verifycode;
		if(A.value==""||str_uintip==A.value){
			A.focus();
			return 
		}
		if(D.value==""){
			D.focus();return 
		}
		if(E.value==""){
			E.focus()
		}
	}catch(C){}
}

function getCookie(D){
	var B=D+"=";
	var F=B.length;
	var A=document.cookie.length;
	var E=0;
	while(E<A){
		var C=E+F;
		if(document.cookie.substring(E,C)==B){
			return getCookieVal(C)
		}
		E=document.cookie.indexOf(" ",E)+1;
		if(E==0){
			break
		}
	}
	return null
}

function getCookieVal(B){
	var A=document.cookie.indexOf(";",B);
	if(A==-1){
		A=document.cookie.length
	}
	return unescape(document.cookie.substring(B,A))
}

function ajax_Submit(){
	var D=true;
	var E=document.forms[0];
	var B="";
	for(var A=0;A<E.length;A++){
		if(E[A].name=="fp"||E[A].type=="submit"){
			continue
		}
		if(E[A].name=="ptredirect"){
			g_ptredirect=E[A].value
		}
		if(E[A].name=="low_login_enable"&&(!E[A].checked)){
			D=false;
			continue
		}
		if(E[A].name=="low_login_hour"&&(!D)){
			continue
		}
		if(E[A].name=="webqq_type"&&(!E[A].checked)){
			continue
		}
		B+=E[A].name;
		B+="=";
		if(t_appid==g_appid
				&&E[A].name=="u"
				&&E[A].value.indexOf("@")<0
				&&isNaN(E[A].value)){
			B+="@"+E[A].value+"&";
			continue
		}
		if(E[A].name=="p"){
			var F="";
			F+=E.verifycode.value;
			F=F.toUpperCase();
			B+=md5(md5_3(E.p.value)+F);
			if(pt.isHttps){
				E.p.value=md5(md5_3(E.p.value)+F)
			}
		}else{
			if(E[A].name=="u1"||E[A].name=="ep"){
				B+=encodeURIComponent(E[A].value)
			}else{
				B+=E[A].value
			}
		}
		B+="&"
	}
	B+="fp=loginerroralert";
	if(pt.isHttps){
		return true
	}
	var C=document.createElement("script");
	C.src=E.action+"?"+B;
	document.cookie="login_param="+encodeURIComponent(login_param)
		+";domain=ui.ptlogin2."
		+g_domain+";path=/";
	document.body.appendChild(C);
	return 
}


function ptuiCB(C,A,B,G,F){$("p").blur();g_time.time13=new Date();var E={"15":g_time.time13-g_time.time12};ptui_speedReport(E);first=false;if(B!=""){switch(G){case"0":window.location.href=B;break;case"1":top.location.href=B;break;case"2":parent.location.href=B;break;default:top.location.href=B}return }g_submitting=false;if(C==0){top.location=document.forms[0].ul.value;return }else{if(A==0){if(F&&F!=""){pt.show_err(F)}else{pt.show_err("您的输入有误，请重试。")}}else{pt.show_err(F);$("p").value="";$("p").focus();$("p").select()}if(isLoadVC){ptui_changeImg(g_domain,g_appid,true);$("verifycode").value="";loadVC(true);$("verifycode").focus();$("verifycode").select()}else{if(A==0){g_uin=0}}if(C==3||C==4){if(navigator.userAgent.toLowerCase().indexOf("webkit")>-1){$("u").focus()}if(C==3){$("p").value=""}$("p").focus();$("p").select();if(C==4){try{$("verifycode").focus();$("verifycode").select()}catch(D){}}if(A!=0){$("verifycode").value="";loadVC(true);g_submitting=true}}}}

function browser_version(){var A=navigator.userAgent.toLowerCase();return A.match(/msie ([\d.]+)/)?1:A.match(/firefox\/([\d.]+)/)?3:A.match(/chrome\/([\d.]+)/)?5:A.match(/opera.([\d.]+)/)?9:A.match(/version\/([\d.]+).*safari/)?7:1}

function ptui_reportSpeed(D,C){if(pt.isHttps){return }if(Math.random()>0.001){return }var A=browser_version();url="http://isdspeed.qq.com/cgi-bin/r.cgi?flag1=6000&flag2=1&flag3="+A;for(var B=0;B<g_speedArray.length;B++){url+="&"+g_speedArray[B][0]+"="+(g_speedArray[B][1]-D)}if(C!=0){url+="&4="+(D-C)}imgSendTimePoint=new Image();imgSendTimePoint.src=url}

function ptui_VCReport(){if(Math.random()>0.001){return }if(g_imgTime<1000000000000){return }var A=browser_version();url="http://isdspeed.qq.com/cgi-bin/r.cgi?flag1=6000&flag2=1&flag3="+A;url+="&5="+g_checkTime+"&6="+g_imgTime}

function ptui_imgPoint(){if(g_imgTime>0){return }g_imgTime=new Date()-g_imgTime;ptui_VCReport()}

function ptui_reportAttr(A){if(Math.random()>0.001){return }url="http://ui.ptlogin2."+g_domain+"/cgi-bin/report?id="+A;imgAttr=new Image();imgAttr.src=url}

function ptui_reportNum(B){if(Math.random()>0.001){return }url="http://ui.ptlogin2."+g_domain+"/cgi-bin/report?id=1000&n="+B;var A=new Image();A.src=url}

function imgLoadReport(){if(changeimg){return }var A=new Date();var B={};if(defaultuin!=""&&g_changeNum<=1){g_time.time8=A;if(!xuiFrame&&defaultuin!=""){B["9"]=g_time.time8-g_time.time1;B["10"]=g_time.time8-g_time.time3;B["11"]=g_time.time8-g_time.time7}}else{g_time.time11=A;B["14"]=g_time.time11-g_time.time10}ptui_speedReport(B)}

function webLoginReport(){var D={};D["7"]=g_time.time3-g_time.time1;if(!xuiFrame){D["8"]=g_time.time4-g_time.time1}try{if(location.hash){var B=location.hash.substring(1,location.hash.length);if(B.indexOf("_")>-1){var A=B.split("_");g_time.time0=A[1]>=A[0]?A[0]:A[1];g_time.time2=A[1]>=A[0]?A[1]:A[0]}else{g_time.time2=B}}}catch(C){}if(g_time.time2&&g_time.time2>0){D["18"]=g_time.time1-g_time.time2;D["21"]=g_time.time3-g_time.time2;if(D["18"]>60000||D["21"]>60000){return }}if(g_time.time0&&g_time.time0>0){D["19"]=g_time.time2-g_time.time0;D["20"]=g_time.time3-g_time.time0;if(D["19"]>60000||D["20"]>60000){return }}ptui_speedReport(D)}

function ptui_speedReport(E){if(pt.isHttps){return }if(!first){return }if(Math.random()>0.001){return }var B="http://isdspeed.qq.com/cgi-bin/r.cgi?flag1=6000&flag2=1&flag3="+browser_version();var C=0;for(var D in E){B+="&"+D+"="+E[D];C++}if(C==0){return }var A=new Image();A.src=B}

function ptui_notifyClose(){try{if(parent.ptlogin2_onClose){parent.ptlogin2_onClose()}else{if(top==this){window.close()}}}catch(A){window.close()}}

function ptui_setUinColor(D,B,A){var C=$(D);if(str_uintip==C.value){C.style.color=A}else{C.style.color=B}}

function ptui_checkPwdOnInput(){if($("p").value.length>=16){return false}return true}

function ptui_onLogin(A){
	try{
		if(parent.ptlogin2_onLogin){
			if(!parent.ptlogin2_onLogin()){
				return false
			}
		}
		if(parent.ptlogin2_onLoginEx){
			var D=A.u.value;
			var B=A.verifycode.value;
			if(str_uintip==D){
				D=""
			}
			if(!parent.ptlogin2_onLoginEx(D,B)){
				return false
			}
		}
	}catch(C){}
	return ptui_checkValidate(A)
}

function ptui_onLoginEx(B,C){
	g_time.time12=new Date();
	if(ptui_onLogin(B)){
		var A=new Date();
		A.setHours(A.getHours()+24*30);
		if((g_appid==t_appid)&&isNaN(B.u.value)&&(B.u.value.indexOf("@")<0)){
			setCookie("ptui_loginuin2",B.u.value,A,"/","ui.ptlogin2."+C)
		}else{
			setCookie("ptui_loginuin",B.u.value,A,"/","ui.ptlogin2."+C)
		}
		if(pt.isHttps){
			return true
		}
	}
	return false
}

function ptui_onReset(A){try{if(parent.ptlogin2_onReset){if(!parent.ptlogin2_onReset()){return false}}}catch(B){}return true}

function ptui_checkValidate(B){
	var A=B.u;
	var D=B.p;
	var E=B.verifycode;
	if(A.value==""||str_uintip==A.value){
		pt.show_err(str_no_uin);
		A.focus();
		return false
	}
	A.value=ptui_trim(A.value);
	if(!ptui_checkQQUin(A.value)){
		pt.show_err(str_inv_uin);
		A.focus();
		A.select();
		return false
	}
	if(D.value==""){
		pt.show_err(str_no_pwd);
		D.focus();
		return false
	}
	if(E.value==""){
		if(!isLoadVC){
			loadVC(true);
			g_submitting=true;
			return false
		}
		pt.show_err(str_no_vcode);
		try{E.focus()}catch(C){}
		if(!g_loadcheck){
			ptui_reportAttr(78028)
		}else{
			ptui_reportAttr(78029)
		}
		return false
	}if(E.value.length<4){
		pt.show_err(str_inv_vcode);
		E.focus();
		E.select();
		return false
	}
	D.setAttribute("maxlength","32");
	ajax_Submit();
	ptui_reportNum(g_changeNum);
	g_changeNum=0;
	return true
}

function setCookie(C,E){var A=setCookie.arguments;var H=setCookie.arguments.length;var B=(2<H)?A[2]:null;var G=(3<H)?A[3]:null;var D=(4<H)?A[4]:null;var F=(5<H)?A[5]:null;document.cookie=C+"="+escape(E)+((B==null)?" ":(";expires ="+B.toGMTString()))+((G==null)?"  ":(";path = "+G))+((D==null)?" ":(";domain ="+D))+((F==true)?";secure":" ")}

var hexcase=1;
var b64pad="";
var chrsz=8;
var mode=32;

function preprocess(A){
	var B="";
	B+=A.verifycode.value;
	B=B.toUpperCase();
	A.p.value=md5(md5_3(A.p.value)+B);
	return true
}

function md5_3(B){
	var A=new Array;
	A=core_md5(str2binl(B),B.length*chrsz);
	A=core_md5(A,16*chrsz);
	A=core_md5(A,16*chrsz);
	return binl2hex(A)
}

function md5(A){
	return hex_md5(A)
}

function hex_md5(A){
	return binl2hex(core_md5(str2binl(A),A.length*chrsz))
}

function str_md5(A){
	return binl2str(core_md5(str2binl(A),A.length*chrsz))
}

function hex_hmac_md5(A,B){
	return binl2hex(core_hmac_md5(A,B))
}

function b64_hmac_md5(A,B){
	return binl2b64(core_hmac_md5(A,B))
}

function str_hmac_md5(A,B){
	return binl2str(core_hmac_md5(A,B))
}
function md5_vm_test(){
	return hex_md5("abc")=="900150983cd24fb0d6963f7d28e17f72"
}

function core_md5(K,F){
	K[F>>5]|=128<<((F)%32);
	K[(((F+64)>>>9)<<4)+14]=F;
	var J=1732584193;
	var I=-271733879;
	var H=-1732584194;
	var G=271733878;
	for(var C=0;C<K.length;C+=16){
		var E=J;
		var D=I;
		var B=H;
		var A=G;
		J=md5_ff(J,I,H,G,K[C+0],7,-680876936);
		G=md5_ff(G,J,I,H,K[C+1],12,-389564586);
		H=md5_ff(H,G,J,I,K[C+2],17,606105819);
		I=md5_ff(I,H,G,J,K[C+3],22,-1044525330);
		J=md5_ff(J,I,H,G,K[C+4],7,-176418897);
		G=md5_ff(G,J,I,H,K[C+5],12,1200080426);
		H=md5_ff(H,G,J,I,K[C+6],17,-1473231341);
		I=md5_ff(I,H,G,J,K[C+7],22,-45705983);
		J=md5_ff(J,I,H,G,K[C+8],7,1770035416);
		G=md5_ff(G,J,I,H,K[C+9],12,-1958414417);
		H=md5_ff(H,G,J,I,K[C+10],17,-42063);
		I=md5_ff(I,H,G,J,K[C+11],22,-1990404162);
		J=md5_ff(J,I,H,G,K[C+12],7,1804603682);
		G=md5_ff(G,J,I,H,K[C+13],12,-40341101);
		H=md5_ff(H,G,J,I,K[C+14],17,-1502002290);
		I=md5_ff(I,H,G,J,K[C+15],22,1236535329);
		J=md5_gg(J,I,H,G,K[C+1],5,-165796510);
		G=md5_gg(G,J,I,H,K[C+6],9,-1069501632);
		H=md5_gg(H,G,J,I,K[C+11],14,643717713);
		I=md5_gg(I,H,G,J,K[C+0],20,-373897302);
		J=md5_gg(J,I,H,G,K[C+5],5,-701558691);
		G=md5_gg(G,J,I,H,K[C+10],9,38016083);
		H=md5_gg(H,G,J,I,K[C+15],14,-660478335);
		I=md5_gg(I,H,G,J,K[C+4],20,-405537848);
		J=md5_gg(J,I,H,G,K[C+9],5,568446438);
		G=md5_gg(G,J,I,H,K[C+14],9,-1019803690);
		H=md5_gg(H,G,J,I,K[C+3],14,-187363961);
		I=md5_gg(I,H,G,J,K[C+8],20,1163531501);
		J=md5_gg(J,I,H,G,K[C+13],5,-1444681467);
		G=md5_gg(G,J,I,H,K[C+2],9,-51403784);
		H=md5_gg(H,G,J,I,K[C+7],14,1735328473);
		I=md5_gg(I,H,G,J,K[C+12],20,-1926607734);
		J=md5_hh(J,I,H,G,K[C+5],4,-378558);
		G=md5_hh(G,J,I,H,K[C+8],11,-2022574463);
		H=md5_hh(H,G,J,I,K[C+11],16,1839030562);
		I=md5_hh(I,H,G,J,K[C+14],23,-35309556);
		J=md5_hh(J,I,H,G,K[C+1],4,-1530992060);
		G=md5_hh(G,J,I,H,K[C+4],11,1272893353);
		H=md5_hh(H,G,J,I,K[C+7],16,-155497632);
		I=md5_hh(I,H,G,J,K[C+10],23,-1094730640);
		J=md5_hh(J,I,H,G,K[C+13],4,681279174);
		G=md5_hh(G,J,I,H,K[C+0],11,-358537222);
		H=md5_hh(H,G,J,I,K[C+3],16,-722521979);
		I=md5_hh(I,H,G,J,K[C+6],23,76029189);
		J=md5_hh(J,I,H,G,K[C+9],4,-640364487);
		G=md5_hh(G,J,I,H,K[C+12],11,-421815835);
		H=md5_hh(H,G,J,I,K[C+15],16,530742520);
		I=md5_hh(I,H,G,J,K[C+2],23,-995338651);
		J=md5_ii(J,I,H,G,K[C+0],6,-198630844);
		G=md5_ii(G,J,I,H,K[C+7],10,1126891415);
		H=md5_ii(H,G,J,I,K[C+14],15,-1416354905);
		I=md5_ii(I,H,G,J,K[C+5],21,-57434055);
		J=md5_ii(J,I,H,G,K[C+12],6,1700485571);
		G=md5_ii(G,J,I,H,K[C+3],10,-1894986606);
		H=md5_ii(H,G,J,I,K[C+10],15,-1051523);
		I=md5_ii(I,H,G,J,K[C+1],21,-2054922799);
		J=md5_ii(J,I,H,G,K[C+8],6,1873313359);
		G=md5_ii(G,J,I,H,K[C+15],10,-30611744);
		H=md5_ii(H,G,J,I,K[C+6],15,-1560198380);
		I=md5_ii(I,H,G,J,K[C+13],21,1309151649);
		J=md5_ii(J,I,H,G,K[C+4],6,-145523070);
		G=md5_ii(G,J,I,H,K[C+11],10,-1120210379);
		H=md5_ii(H,G,J,I,K[C+2],15,718787259);
		I=md5_ii(I,H,G,J,K[C+9],21,-343485551);
		J=safe_add(J,E);
		I=safe_add(I,D);
		H=safe_add(H,B);
		G=safe_add(G,A)
	}
	if(mode==16){
		return Array(I,H)
	}else{
		return Array(J,I,H,G)
	}
}

function md5_cmn(F,C,B,A,E,D){
	return safe_add(bit_rol(safe_add(safe_add(C,F),safe_add(A,D)),E),B)
}

function md5_ff(C,B,G,F,A,E,D){return md5_cmn((B&G)|((~B)&F),C,B,A,E,D)}
function md5_gg(C,B,G,F,A,E,D){return md5_cmn((B&F)|(G&(~F)),C,B,A,E,D)}
function md5_hh(C,B,G,F,A,E,D){return md5_cmn(B^G^F,C,B,A,E,D)}
function md5_ii(C,B,G,F,A,E,D){return md5_cmn(G^(B|(~F)),C,B,A,E,D)}

function core_hmac_md5(C,F){
	var E=str2binl(C);
	if(E.length>16){
		E=core_md5(E,C.length*chrsz)
	}
	var A=Array(16),D=Array(16);
	for(var B=0;B<16;B++){
		A[B]=E[B]^909522486;
		D[B]=E[B]^1549556828
	}
	var G=core_md5(A.concat(str2binl(F)),512+F.length*chrsz);
	return core_md5(D.concat(G),512+128)
}

function safe_add(A,D){
	var C=(A&65535)+(D&65535);
	var B=(A>>16)+(D>>16)+(C>>16);
	return(B<<16)|(C&65535)
}

function bit_rol(A,B){
	return(A<<B)|(A>>>(32-B))
}
function str2binl(D){
	var C=Array();
	var A=(1<<chrsz)-1;
	for(var B=0;B<D.length*chrsz;B+=chrsz){
		C[B>>5]|=(D.charCodeAt(B/chrsz)&A)<<(B%32)
	}
	return C
}
function binl2str(C){
	var D="";
	var A=(1<<chrsz)-1;
	for(var B=0;B<C.length*32;B+=chrsz){
		D+=String.fromCharCode((C[B>>5]>>>(B%32))&A)
	}
	return D
}
function binl2hex(C){
	var B=hexcase?"0123456789ABCDEF":"0123456789abcdef";
	var D="";
	for(var A=0;A<C.length*4;A++){
		D+=B.charAt((C[A>>2]>>((A%4)*8+4))&15)+B.charAt((C[A>>2]>>((A%4)*8))&15)
	}
	return D
}
function binl2b64(D){
	var C="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	var F="";
	for(var B=0;B<D.length*4;B+=3){
		var E=(((D[B>>2]>>8*(B%4))&255)<<16)
			|(((D[B+1>>2]>>8*((B+1)%4))&255)<<8)
			|((D[B+2>>2]>>8*((B+2)%4))&255);
			
		for(var A=0;A<4;A++){
			if(B*8+A*6>D.length*32){
				F+=b64pad
			}else{
				F+=C.charAt((E>>6*(3-A))&63)
			}
		}
	}
	return F
}

isAbleSubmit=true;
function check(){var B=new Date();if(defaultuin!=""&&g_changeNum==0){g_time.time6=B}else{g_time.time9=B}var A=ptui_trim($("u").value);if(g_uin==A||(!ptui_checkQQUin(A))){return }g_changeNum++;g_uin=ptui_trim($("u").value);try{if(parent.ptui_uin){parent.ptui_uin(g_uin)}}catch(C){}ptui_needVC(g_uin,g_appid)}
function loadVC(A){if(isLoadVC==A&&(lastUin==g_uin)){return }lastUin=g_uin;isLoadVC=A;if(A==true){var B=$("imgVerify");var F=g_uin;if((g_appid==t_appid)&&isNaN(g_uin)&&(g_uin.indexOf("@")<0)){F="@"+g_uin}var E="/getimage?aid="+g_appid+"&r="+Math.random()+"&uin="+F;if(g_https){E="."+E}else{E="http://captcha."+g_domain+E+"&vc_type="+vc_type}var D=new Date();B.src=E;$("verifyinput").style.display="";$("verifytip").style.display="";$("verifyshow").style.display="";ptui_notifySize("login");try{$("p").focus()}catch(C){}}else{$("verifyinput").style.display="none";$("verifytip").style.display="none";$("verifyshow").style.display="none";ptui_notifySize("login");try{$("p").focus()}catch(C){}}}

function onPageClose(){ptui_notifyClose()}
function onFormReset(A){if(ptui_onReset(A)){A.u.style.color="#CCCCCC";return true}return false}
function onClickForgetPwd(){var B=$("u");var A=$("label_forget_pwd");A.href=g_forget;if(B!=null&&B.value!=str_uintip){if(A.href.indexOf("?")==-1){A.href+="?"}else{A.href+="&"}A.href+="aquin="+B.value}return true}

function $_(A){return document.getElementsByTagName(A)}
var stat_list=["im.qq.com下","http://ui.ptlogin2.qq.com/","http://xui.ptlogin2.qq.com/","http://emailreg.qq.com/","http://cr.sec.qq.com/","http://imgcache.qq.com/","http://gtimg.cn/","http://gtimg.com/","http://static.paipaiimg.com/","http://cache.idqqimg.com"];
function stat(A){if(typeof (A)=="undefined"){return true}A+="";if(A==""){return true}for(var B=0;B<stat_list.length;B++){if(A.indexOf(stat_list[B])>-1){return true}}if(A.indexOf("http://ui.ptlogin2."+g_domain+"/")>-1){return true}if(A.indexOf("http://ptlogin2."+g_domain+"/")>-1){return true}if((A.indexOf(g_domain)>-1)&&(A.indexOf("http")>-1)){var C=A.substring(7,A.indexOf(g_domain));if(C.indexOf("/")==-1){return true}}return false}

var scripts=$_("SCRIPT");
var iframes=$_("IFRAME");
var frames=$_("FRAME");
var forms=$_("FORM");
var secVer="1.2";
function static_rpt(H){
var K=/https?:\/\/\d+\.\d+\.\d+\.\d+.*?[\s\"\']/g;
var B=/https?:\/\/.+?[\s\"\']/g;
var D=new Array();
var A;
for(var C=0;C<scripts.length;C++){
	while(A=K.exec(scripts[C].innerHTML)){
		D.push("scripts::"+encodeURIComponent(A))
	}
	while(A=B.exec(scripts[C].innerHTML)){
		if(!stat(A)){
			D.push("scripts::"+encodeURIComponent(A))
		}
	}
}
for(var C=0;C<scripts.length;C++){
	A=scripts[C].src;
	if(!stat(A)){
		D.push("scripts::"+encodeURIComponent(A))
	}
}
for(var C=0;C<iframes.length;C++){A=iframes[C].src;if(!stat(A)){D.push("iframes::"+encodeURIComponent(A))}}for(var C=0;C<frames.length;C++){A=frames[C].src;if(!stat(A)){D.push("frames::"+encodeURIComponent(A))}}for(var C=0;C<forms.length;C++){A=forms[C].action;if(!stat(A)){D.push("forms::"+encodeURIComponent(A))}}try{if(D.length>0){if(pt.isHttps){return }if(H){D.push(encodeURIComponent(window.location.href));I="datapt"}else{D.push(encodeURIComponent(parent.window.location.href));I="datapp"}var E=D.join("|");var I;var J="http://cr.sec.qq.com/cr?id=5&d="+I+"=v"+secVer+"|"+E;var G=new Image();G.src=J}}catch(F){}}

static_rpt(true);
try{if(parent!=window){scripts=parent.$_("SCRIPT");iframes=parent.$_("IFRAME");frames=parent.$_("FRAME");forms=parent.$_("FORM");static_rpt(false)}}catch(e){};/*  |xGv00|eaa967eeb44f408aa6535efd5ff763f1 */
