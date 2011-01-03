Jet().$package(
	"qqweb",
	function(f){
		var d;
		document.domain="qq.com";
		d=window.location.host;
		window.onerror=function(){
			return true
		};
		this.init=function(){
			f.$namespace("qqweb.app");
			this.portal.init({});
			qqweb.portal.speedTest.sRTS(7,"start"
						,window._SPEEDTIME_WINDOWSTART);
			qqweb.portal.speedTest.sRTS(7,"end",new Date,true);
			qqweb.portal.speedTest.sRTS(8,"start",new Date)
		};
		this.CONST={
			CDN_URL:"http://hp.qq.com/webqqpic/",
			UPDATE_TIME_STAMP:"20101231001",
			MAIN_DOMAIN:"qq.com",
			DOMAIN:d,
			MAIN_URL:"http://"+d+"/",
			API_SERVER_URL:"http://s.web2.qq.com/api/",
			CONN_SERVER_DOMAIN:"http://s.web2.qq.com/",
			CONN_SERVER_DOMAIN2:"http://d.web2.qq.com/",
			CGI_BIN_SERVER_URL:"http://web2-b.qq.com/cgi-bin/",
			CGI_BIN_SERVER_URL2:"http://"+d+"/cgi-bin/",
			CGI_BIN_SERVER_URL3:"http://web.qq.com/cgi-bin/",
			API_PROXY_URL:"http://s.web2.qq.com/proxy.html?v=20101025002",
			PUB_APP_STATIC_URL:"./pubapps/",
			PRI_APP_STATIC_URL:"http://wqbg.qpic.cn/appmarket/",
			PRI_APP_STATIC_URL2:"./",
			DEFAULT_AVATAR_URL:"./style/",
			AVATAR_SERVER_DOMAIN:"http://qun.qq.com/",
			AVATAR_SERVER_DOMAINS:[
				"http://face1.qun.qq.com/",
				"http://face2.qun.qq.com/",
				"http://face3.qun.qq.com/",
				"http://face4.qun.qq.com/",
				"http://face5.qun.qq.com/",
				"http://face6.qun.qq.com/",
				"http://face7.qun.qq.com/",
				"http://face8.qun.qq.com/",
				"http://face9.qun.qq.com/",
				"http://face10.qun.qq.com/",
				"http://face11.qun.qq.com/"],
			QZONE_SERVER_DOMAIN:"http://qzone.qq.com/",
			QZONE_USER_SERVER_DOMAIN:"http://user.qzone.qq.com/",
			QMAIL_SERVER_DOMAIN:"http://mail.qq.com/",
			MAX_LOGIN_AMOUNT:1,
			MAX_FAIL_AMOUNT:2,
			LOAD_AVATAR_AMOUNT:50,
			LOGIN_LEVEL_NONE:1,
			LOGIN_LEVEL_NOCHAT:2,
			LOGIN_LEVEL_ALL:3,
			KET:0.1,
			WINDOW_FLAG_MIN:1,
			WINDOW_FLAG_NORMAL:2,
			WINDOW_FLAG_MAX:4,
			WINDOW_FLAG_CURRENT:8,
			WINDOW_FLAG_NOT_CURRENT:16,
			WINDOW_FLAG_FULLSCREEN:32
		}
	}
);

function ptlogin2_onResize(f,d){}
lockedEl=null;
padEventProxy=function(f,d){
	var b,a;
	d.initEvent(f,true,false);
	if(d.changedTouches&&d.changedTouches.length){
		a=d.changedTouches[0];
		b=a.pageX;
		a=a.pageY
	}else{
		b=d.clientX;
		a=d.clientY
	}
	if(f=="touchmove")
		b=lockedEl?lockedEl:(lockedEl=document.elementFromPoint(b,a));
	else if(lockedEl&&(f=="touchend"||f=="touchcancel")){
		b=lockedEl;lockedEl=null
	}else 
		b=document.elementFromPoint(b,a);
	a=qqweb.layout.getCurrentWindow();
	if(b.tagName=="IFRAME"&&a){
		a=document.getElementById("iframeApp_"+a.getId());
		var h=false;
		try{
			h=a&&typeof a.contentWindow.padEventProxy=="function"
				?true:false
		}catch(s){}
		h?a.contentWindow.padEventProxy(f,d):b.dispatchEvent(d)
	}else 
		b.dispatchEvent(d)
};

Jet().$package(
	"qqweb.util",
	function(f){
		var d=f.dom,b=f.browser;
		this.observer={
			openInWebBrowser:function(a){
				try{
					a.preventDefault()
				}catch(h){}
				a=this.getAttribute("href");
				var s=this.getAttribute("title");
				qqweb.portal.runApp(
					"6"
					,{url:a,isHideBar:false,title:s}
				)
			}
		};
		this.getUserDefaultAvatar=function(a){
			a=a||40;
			return"./style/images/avatar_default_"+a+"_"+a+".gif"
		};
		this.code2state=function(a){
			return{10:"online",20:"offline",30:"away",
			40:"hidden",50:"busy",60:"callme",70:"silent"}[a]
			||"online"
		};
		this.getFaceServer=function(a){
			return qqweb.CONST.AVATAR_SERVER_DOMAINS[a%10]
		};
		this.getUserAvatar=function(a,h){
			h=h||0;
			if(isNaN(a))
				return this.getDefaultUserAvatar();
			return this.getFaceServer(a)
				+"cgi/svr/face/getface?cache="
				+h+"&type=1&fid=0&uin="
				+a+"&vfwebqq="+qqweb.portal.getVfWebQQ()
			};
		this.getGroupAvatar=function(a,h){
			h=h||0;
			return this.getFaceServer(a)
				+"cgi/svr/face/getface?cache="
				+h+"&type=4&fid=0&uin="
				+a+"&vfwebqq="
				+qqweb.portal.getVfWebQQ()
		};
		this.getQzoneUrl=function(a){
			return qqweb.CONST.QZONE_USER_SERVER_DOMAIN+a
		};
		this.getSendMailUrl=function(a){
			return"http://mail.qq.com/cgi-bin/login?"
				+"Fun=clientwrite&vm=pt&email="
				+a+"@qq.com"
		};
		this.getDefaultUserAvatar=function(){
			return"./style/images/avatar.png"
		};
		this.setDefaultAppThumb=function(a){
			a.src="./style/images/thumb_default.png"
		};
		this.IEAddOption=function(a,h){
			if(b.ie){
				var s=d.node("option"
					,{value:h.value,text:h.text});
				if(h.selected)
					s.selected="selected";
				a.options.add(s)
			}
		};
		this.setPngForIE6=function(a,h){
			if(f.browser.ie==6){
				a.style.background="none";
				a.style.filter="progid:DXImageTransform."
				+"Microsoft.AlphaImageLoader(src='"
				+h+"', sizingMethod='crop')"
			}
		};
		this.getFileSize=function(a){
			var h=new Image,s=a.value,w=0;
			try{h.dynsrc=s}catch(c){return 0}
			try{w=h.fileSize||0}catch(i){}
			if(w==0)
				try{w=a.files[0].fileSize}catch(n){}
			return w
		}
	}
);

Jet().$package(
	"qqweb.config",
	function(f){
		var d=this,b=f.event,a=f.dom;
		b=f.event;
		var h=f.string,s=false,w,c,i,n;
		w=[50,51,2,17,16,6];
		c=[50,51,2,17,16,7,21,28,5,1,45,14,29,34
			,8,30,47,46,12,15,24,48,42,49,9,26
			,27,36,35,37,31,39,38,22,11,6,13
			,4,10,33,40,3,18,20,32];
		i=[
			{id:0,name:"\u793e\u4ea4",list:[50,51,2,17,16,7,21,28,5,1]},
			{id:1,name:"\u751f\u6d3b",list:[45,14,29,34,8,30,47,46]},
			{id:2,name:"\u5a31\u4e50",list:[12,15,24,48,42,49,9,26,27,36
							,35,37,31,39,38,22,11]},
			{id:3,name:"\u5de5\u5177",list:[6,13,4,10,33,40,3,18,20,32]},
			{id:4,name:"\u81ea\u5b9a\u4e49",list:[]}
		];
		n={"1":0,"2":0,"3":3,"4":3,"5":0,"6":3,"7":0,"8":1,"9":2
			,"10":3,"11":2,"12":2,"13":3,"14":1,"15":2,"16":0,"17":0
			,"18":3,"19":4,"20":3,"21":0,"22":2,"23":4,"24":2,"25":4
			,"26":2,"27":2,"28":0,"29":1,"30":1,"31":2,"32":3,"33":3
			,"34":1,"35":2,"36":2,"37":2,"38":2,"39":2,"40":3,"41":4
			,"42":2,"43":4,"44":4,"45":1,"46":4,"47":4,"48":4,"49":4
			,"50":0,"51":0,"56":2,"58":2,"59":2,"61":1,"62":1};
		if(f.browser.mobileSafari){
			c=[50,51,2,17,16,7,21,28,5,1,45,14,29,34,8,30,47,46
				,56,12,15,24,48,42,49,9,26,27,36,35,37,31,39
				,38,22,11,6,13,4,10,33,40,3,18,20,32];
			i=[
				{id:0,name:"\u793e\u4ea4",list:[50,51,2,17,16,7,21,28,5,1]},
				{id:1,name:"\u751f\u6d3b",list:[45,14,29,34,8,30,47,46]},
				{id:2,name:"\u5a31\u4e50",list:[56,12,15,24,48,42,49,9,26,27
						,36,35,37,31,39,38,22,11]},
				{id:3,name:"\u5de5\u5177",list:[6,13,4,10,33,40,3,18,20,32]},
				{id:4,name:"\u81ea\u5b9a\u4e49",list:[]}]
		}
		this.configList={
			theme:{id:"theme_2011"},
			wallpaper:{id:"",mode:"repeat",url:""},
			wallpaperList:[],
			appearance:{id:"black"},
			appBarSetting:{},
			quickAppList:w,
			folderList:i,
			defaultSetupAppList:c,
			setupAppList:c
		};
		this.onSetConfig=function(){};
		this.onConfigGetSuc=function(e){
			qqweb.portal.speedTest.sRTS(4,"end",new Date,true);
			e=e.result&&e.result.app?e.result.app:[];
			var l=0;
			for(var m in e)
				if(m==="QQWeb"){
					var o=e[m];
					if(o.theme&&o.theme!="")
						this.configList.theme.id=o.theme;
					if(o.wallpaper&&o.wallpaper!=""&&o.wallpaper.id!="")
						this.configList.wallpaper=o.wallpaper;
					if(o.wallpaperList&&o.wallpaperList!=
"")
this.configList.wallpaperList=o.wallpaperList;if(o.appearance&&o.appearance!="")this.configList.appearance.id=o.appearance;if(o.runStatus)this.configList.runStatus=o.runStatus;if(o.chatboxMode)this.configList.chatboxMode=o.chatboxMode;if(o.isNotNeedCtrlKey)this.configList.isNotNeedCtrlKey=o.isNotNeedCtrlKey;if(o.isShowTip)this.configList.isShowTip=o.isShowTip;if(o.fontFormat)this.configList.fontFormat=o.fontFormat;if(o.appBarSetting)this.configList.appBarSetting=o.appBarSetting;if(o.notifySetting)this.configList.notifySetting=
o.notifySetting;if(o.msgBubblePos)this.configList.msgBubblePos=o.msgBubblePos;if(!o.setupAppList||!f.isNumber(o.setupAppList[0])){var v={onSuccess:function(){},context:this,data:{retype:1,app:"QQWeb",itemlist:f.json.stringify({setupAppList:this.getSetupAppList()})}};qqweb.rpcService.sendSetConfig(v)}else this.configList.setupAppList=o.setupAppList.length==0?[]:o.setupAppList;s=true;if(o.folderList)this.configList.folderList=o.folderList;else{for(l in this.configList.folderList)this.configList.folderList[l].list=
[];for(l in this.configList.setupAppList){v=this.configList.setupAppList[l];var z=this.getFolderIndexByFolderId(4);f.isUndefined(n[v])?this.configList.folderList[z].list.push(parseInt(v)):this.configList.folderList[n[v]].list.push(parseInt(v))}v={onSuccess:function(){},context:this,data:{retype:1,app:"QQWeb",itemlist:f.json.stringify({folderList:this.getFolderList()})}};qqweb.rpcService.sendSetConfig(v)}qqweb.layout.initSystemTheme();this.checkAndInstall();if(o.quickAppList)this.configList.quickAppList=
o.quickAppList;else{this.configList.quickAppList=[];for(l in w)f.array.indexOf(this.getSetupAppList(),w[l])!==-1&&this.configList.quickAppList.push(w[l]);v={onSuccess:function(){},context:this,data:{retype:1,app:"QQWeb",itemlist:f.json.stringify({quickAppList:this.getQuickAppList()})}};qqweb.rpcService.sendSetConfig(v)}}m=qqweb.portal.getLoginLevel();f.out("logininfoSuccess");b.notifyObservers(qqweb.portal,"UserAppListReady",m);s&&b.notifyObservers(this,"GetUserAppListSuccess",this.getSetupAppList())};
this.checkAndInstall=function(){var e=[];e=f.browser.mobileSafari?[50,56]:[50];e=e.reverse();var l=false;for(var m in e){var o=e[m];if(f.array.indexOf(this.configList.setupAppList,o)==-1){this.configList.setupAppList.splice(0,0,o);l=true;var v=(this.getFolderById(n[o])||this.getFolderById(4)).list;f.array.indexOf(v,o)==-1&&v.splice(0,0,o)}}if(l){m={onSuccess:function(){},context:d,data:{retype:1,app:"QQWeb",itemlist:f.json.stringify({setupAppList:d.getSetupAppList(),folderList:this.getFolderList(),
quickAppList:d.getQuickAppList()})}};qqweb.rpcService.sendSetConfig(m)}};this.getAppBarSetting=function(){return this.configList.appBarSetting};this.setAppBarSetting=function(e){this.configList.appBarSetting=e;e={onSuccess:function(){},context:d,data:{retype:1,app:"QQWeb",itemlist:f.json.stringify({appBarSetting:d.getAppBarSetting()})}};qqweb.rpcService.sendSetConfig(e)};var t=function(){if(!(qqweb.portal.getLoginLevel()==1||!s)){var e={onSuccess:function(){},context:d,data:{retype:1,app:"QQWeb",
itemlist:f.json.stringify({setupAppList:d.getSetupAppList()})}};qqweb.rpcService.sendSetConfig(e)}};this.setAppListQueue=function(e){var l=[];for(var m in e)l.push(parseInt(e[m]));this.configList.setupAppList=l;t()};this.add2SetupAppList=function(e){if(this.getSetupAppList().length>=200){var l='<div class="appAlert_container">\t\t\t\t\t\t\t<div style="height:24px;line-height:24px;font-size:12px;text-align:center;margin-top:5px;">\u5e94\u7528\u6dfb\u52a0\u91cf\u6700\u591a\u4e3a200\u4e2a,\u8bf7\u5220\u51cf\u90e8\u5206\u5e94\u7528\u540e\u518d\u6dfb\u52a0\u3002</div>\t\t\t\t\t\t</div>',
m=new qqweb.businessClass.Window({title:"\u6e29\u99a8\u63d0\u793a",modeSwitch:true,dragable:true,resize:false,width:370,height:127,html:l,hasOkButton:true,hasCloseButton:true,isSetCentered:true});m.setTopZIndex()}else if(f.array.indexOf(this.getSetupAppList(),e.id)==-1&&!a.id("appAlert_category_select_"+e.id)){l='<div class="appAlert_container">\t\t\t\t\t\t\t<div class="appAlert_alert">\u60a8\u5c06\u6dfb\u52a0\u3010'+h.encodeHtml(e.appName)+'\u3011\u5e94\u7528</div>\t\t\t\t\t\t\t<div class="appAlert_category">\t\t\t\t\t\t\t\t<span class="appAlert_category_text" id="appAlert_category_text">\u9009\u62e9\u5e94\u7528\u5206\u7ec4\uff1a</span>\t\t\t\t\t\t\t\t<select id="appAlert_category_select_'+
e.id+'" class="appAlert_category_select"></select>\t\t\t\t\t\t\t</div>\t\t\t\t\t\t</div>';m=new qqweb.businessClass.Window({title:"\u6e29\u99a8\u63d0\u793a",modeSwitch:true,dragable:true,resize:false,width:370,height:168,html:l,hasOkButton:true,hasCloseButton:true,hasCancelButton:true,isSetCentered:true});m.setTopZIndex();var o=a.id("appAlert_category_select_"+e.id);l=this.getFolderList();for(var v=0;v<l.length;v++){var z=document.createElement("option");z.value=l[v].id;z.innerHTML=h.encodeHtml(l[v].name);
o&&o.appendChild(z)}o.value=4;b.addObserver(m,"clickOkButton",function(){var H=d.getFolderIndexByFolderId(o.value);d.addToFolderList(H,e.id);d.configList.setupAppList.push(e.id);qqweb.appconfig.addAppConfig(e);t();m.close()});if(e.id<1E5)(l=e.exinfo.reportName)&&f.string.trim(l)&&pgvSendClick({hottag:"WEB2QQ.ADDAPP."+l+".LOGIN"})}};this.removeSetupAppList=function(e){if(e.cannotUninstall)alert("\u62b1\u6b49,\u6b64\u5e94\u7528\u4e0d\u80fd\u5220\u9664!");else{qqweb.appconfig.removeAppConfig(e);this.removeFromFolderListById(e.id);
this.removeFromQuickAppList(e.id);f.array.remove(this.getSetupAppList(),parseInt(e.id));t()}};this.getSetupAppList=function(){return this.configList.setupAppList};this.getDefaultSetupAppList=function(){return this.configList.defaultSetupAppList};this.isSetupAppListLoaded=function(){return s};var x=function(){if(!(qqweb.portal.getLoginLevel()==1||!s)){var e={onSuccess:function(){},context:d,data:{retype:1,app:"QQWeb",itemlist:f.json.stringify({quickAppList:d.getQuickAppList()})}};qqweb.rpcService.sendSetConfig(e)}};
this.getQuickAppList=function(){return this.configList.quickAppList};this.setQuickQppList=function(e){for(var l in e)e[l]=parseInt(e[l]);this.configList.quickAppList=e;x()};this.addToQuickAppList=function(e){var l=e.appId=parseInt(e.appId),m=e.index;if(f.array.indexOf(this.getQuickAppList(),l)==-1){m?this.configList.quickAppList.splice(m,0,l):this.configList.quickAppList.push(l);x();b.notifyObservers(d,"AddToQuickAppList",e);pgvSendClick({hottag:"web2qq.AppBar.wShortcut.creat"})}};this.removeFromQuickAppList=
function(e){if(f.array.indexOf(this.getQuickAppList(),parseInt(e))>-1){f.array.remove(this.getQuickAppList(),parseInt(e));x();b.notifyObservers(d,"RemoveFromQuickAppList",e);pgvSendClick({hottag:"web2qq.AppBar.wShortcut.remove"})}};var j=function(){if(!(qqweb.portal.getLoginLevel()==1||!s)){var e={onSuccess:function(){},context:d,data:{retype:1,app:"QQWeb",itemlist:f.json.stringify({folderList:d.getFolderList()})}};qqweb.rpcService.sendSetConfig(e)}};this.getFolderList=function(e){return f.isUndefined(e)?
this.configList.folderList:this.configList.folderList[e]};this.setFolderList=function(e,l){if(f.isUndefined(l))this.configList.folderList=e;else this.configList.folderList[l]=e};this.getFolderIdById=function(e){var l,m=this.getFolderList();e=parseInt(e);for(var o in m)if(f.array.indexOf(m[o].list,e)>-1){l=m[o].id;break}return parseInt(l)};this.getFolderIndexByFolderId=function(e){var l,m=this.getFolderList();for(var o in m)if(m[o].id==e){l=o;break}return parseInt(l)};this.getFolderById=function(e){var l,
m=this.getFolderList();for(var o in m)if(m[o].id==e)l=m[o];return l};this.removeFromFolderListById=function(e){var l=this.getFolderList();for(var m in l){var o=l[m].list;f.array.indexOf(o,e)>-1&&f.array.remove(o,parseInt(e))}j()};this.removeFolderByFolderId=function(e){if(e==4)alert("\u62b1\u6b49,\u6b64\u6587\u4ef6\u5939\u4e0d\u5141\u8bb8\u5220\u9664!");else{var l=this.getFolderById(e),m=l.list;for(var o in m){f.array.remove(this.getSetupAppList(),parseInt(m[o]));f.array.remove(this.getQuickAppList(),
parseInt(m[o]));var v=qqweb.appconfig.getAppConfig(parseInt(m[o]));qqweb.appconfig.removeAppConfig(v)}f.array.remove(this.getFolderList(),l);t();x();j();b.notifyObservers(d,"RemoveFolderByFolderId",e)}};this.updateFloderName=function(e,l){var m=this.getFolderList();for(var o in m)if(m[o].id==e)m[o].name=l;j()};this.updateFolderList=function(e){for(var l in e){for(var m in e[l])e[l][m]=parseInt(e[l][m]);this.getFolderList(l).list=e[l]}j()};this.addToFolderList=function(e,l){e=this.getFolderList(e).list;
if(f.array.indexOf(e,l)==-1){e.push(l);j()}};this.restoreConfig=function(){};this.getTheme=function(){return this.configList.theme};this.setTheme=function(e){if(e){var l={};l.data={retype:1,app:"QQWeb",itemlist:f.json.stringify({theme:e})};qqweb.rpcService.sendSetConfig(l);this.configList.theme.id=e}};this.getWallpaper=function(){return this.configList.wallpaper};this.setWallpaper=function(e){if(e){var l={};l.data={retype:1,app:"QQWeb",itemlist:f.json.stringify({wallpaper:e})};qqweb.rpcService.sendSetConfig(l);
this.configList.wallpaper=e}};this.getWallpaperList=function(){return this.configList.wallpaperList};this.addWallpaper=function(e){f.array.indexOf(this.configList.setupAppList,e.id)==-1&&this.configList.wallpaperList.push(e.fileId)};this.removeWallpaper=function(e){f.array.remove(this.getWallpaperList(),e.fileId)};this.getAppearance=function(){return this.configList.appearance};this.setAppearance=function(e){if(e){var l={};l.data={retype:1,app:"QQWeb",itemlist:f.json.stringify({appearance:e})};qqweb.rpcService.sendSetConfig(l);
this.configList.appearance.id=e}};this.setCustomTheme=function(e,l){if(e){l=l||"";var m={};m.data={retype:1,app:"QQWeb",itemlist:f.json.stringify({appearance:l,wallpaper:e})};qqweb.rpcService.sendSetConfig(m);this.configList.appearance.id=l;this.configList.wallpaper=e}};this.initQQWeb=function(){var e={onSuccess:qqweb.config.onConfigGetSuc,action:"get_custom",context:this,data:{retype:1,itemlist:f.json.stringify({QQWeb:["theme","wallpaper","wallpaperList","appearance","appBarSetting","setupAppList",
"isShowTip","quickAppList","folderList","runStatus","chatboxMode","isNotNeedCtrlKey","fontFormat","msgBubblePos","notifySetting"]})}};qqweb.rpcService.sendGetConfig(e)};d.__eqqid="50"});

Jet().$package(
	"qqweb.portal",
	function(f){
		var d=this,b=f.dom,a=f.event,h=f.http,
		s,w=false,c=qqweb.CONST.LOGIN_LEVEL_NONE,i=false,n=false,t=false,
		x=false,j="",e=false,l,m,o=null,v=document.title,z=null,H=false,
		P=false,Y=true;
		
		this.speedTest=new (
			function(){
				var g=[];
				this.sRTS=this.setReportTimeStamp=function(k,r,D,F){
					g[k]||(g[k]={});
					g[k][r]=D.getTime();
					F==true&&this.report([k])
				};
				this.gRTS=this.getReportTimeStamp=function(k,r){
					if(g[k])return g[k][r];
					return null
				};
				this.report=function(k){
					for(var r=false,
		D="http://isdspeed.qq.com/cgi-bin/r.cgi?flag1=7723&flag2=2&flag3=1&flag4="
							+qqweb.portal.getCookieUin(),F=0;
							F<k.length;F++){
						var C=k[F];
						if(g[C].end&&g[C].start){
							r=true;
							D+="&"+C+"="
							+(g[C].end-g[C].start)
						}
					}
					if(r)(new Image).src=D
				}
			}
		);
		this.setPortalSelf=function(g){
			qqweb.portal.self.allow=g.allow;qqweb.portal.self.age=g.age;
			qqweb.portal.self.nick=g.nick;
			qqweb.portal.self.htmlNick=f.string.encodeHtml(String(g.nick));
			qqweb.portal.self.titleNick=String(g.nick);
			qqweb.portal.self.country=g.country;
			qqweb.portal.self.province=g.province;
			qqweb.portal.self.city=g.city;
			qqweb.portal.self.gender=g.gender;
			qqweb.portal.self.face=g.face;
			qqweb.portal.self.phone=g.phone;
			qqweb.portal.self.mobile=g.mobile;
			qqweb.portal.self.email=g.email
		};
		this.setPortalSelfItem=function(g,k){
			qqweb.portal.self[g]=k
		};
		this.getPortalSelf=function(g){
			return typeof qqweb.portal.self=="undefined"?{}
				:typeof g=="undefined"?qqweb.portal.self
				:qqweb.portal.self[g]
		};
		var ia=function(g){
			d.runApp("appBar",
					{callback:function(){
						d.runApp("appBar",{
							callback:function(){
								g();
								f.out("tipsAction");
								d.runApp("tips");
								f.out("tipsEnd");
								d.runApp("messageCenter");
								d.runApp("bubbleTip")
							}
						})
					}}
				)
		},
		ja=function(){
			for(var g=d.getDefaultApps(),k=0;k<g.length;++k)
				if(!(d.getLoginLevel()>1&&g[k]==qqweb.config.__eqqid))
					if(f.browser.safari)
						switch(g[k]){
						case "20":break;
						default:d.runApp(g[k])
						}
					else 
						d.runApp(g[k])
		};
		this.getDefaultApps=function(){
			return["18","19","20",qqweb.config.__eqqid]
		};
		var N=function(){return["56"]},
			da=function(){
				for(var g=N(),k=0;k<g.length;++k)
					switch(g[k]){
					case "56":f.browser.mobileSafari&&d.runApp(g[k]);break;
					default:break
					}
			},
			ea=function(){
				var g=f.string.mapQuery(window.location.search).run||"";
				if(g)return 
				f.json.parse(g)
			},
			Z=function(){
				var g=ea();
				for(var k in g){
					g[k].runFrom="url";
					qqweb.portal.runApp(k,g[k])
				}
			},
			T=this.setLoginLevel=function(g){
				c=g;a.notifyObservers(qqweb.portal,"loginLevelChanged",g)
			},
			G={isUserAppListReady:false,isAppbarReady:false},
			Q=function(){
				var g=true;
				for(var k in G)
					G[k]||(g=false);
				if(g){
					f.out("\u7cfb\u7edf\u6a21\u5757Ready");
					a.notifyObservers(d,"systemAppReady")}
			};
			this.getIsUserAppListReady=function(){
				return G.isUserAppListReady
			};
			
			$=function(){
				return j==d.uin?true:false
			},
			
			this.init=function(g){
				g={};s={};l=0;
				a.addObserver(qqweb.portal,"exitSuccess",K.onExitSuccess);
				a.addObserver(qqweb.rpcService,"GetVfWebQQError",K.onGetVfWebQQError);
				a.addObserver(qqweb.rpcService,"GetVfWebQQSuccess",K.onGetVfWebQQSuccess);
				a.addObserver(qqweb.portal,"GetLoginInfoSuccess",K.onGetLoginInfoSuccess);
				a.addObserver(qqweb.appconfig,"GetAppConfigComplete",K.onGetAppConfigComplete);
				a.addObserver(qqweb.appconfig,"GetDefaultAppConfigComplete"
						,K.onGetAppConfigComplete);
				a.addObserver(qqweb.appconfig,"UpdateAppConfig",K.onUpdateAppConfig);
				a.addObserver(qqweb.appconfig,"RemoveAppConfig",K.onRemoveAppConfig);
				a.addObserver(qqweb.portal,"portalReady",K.onPortalReady);
				a.addObserver(qqweb.portal,"UserAppListReady",K.onUserAppListReady);
				a.addObserver(qqweb.portal,"appbarReady",K.onAppbarReady);
				qqweb.sound.init();
				qqweb.util.initSystem();
				ia(
					function(){
						d.start();d.runApp("myPanel");
						qqweb.rpcService.sendGetVfWebQQ(d.uin)
					}
				)
			};
			this.start=function(){
				this.recordAccount()
			};
			this.recordAccount=function(){
				this.ptwebqq=this.getCookiePtwebqq();
				this.uin=this.getCookieUin();
				this.originalUin=this.getOriginalCookieUin();
				this.skey=this.getCookieSkey()
			};
			this.getPtwebqq=function(){
				return this.ptwebqq
			};
			this.setPtwebqq=function(g){
				return this.ptwebqq=g
			};
			this.getUin=function(){
				return this.uin
			};
			this.getOriginalUin=function(){
				return this.originalUin
			};
			this.getSkey=function(){
				return this.skey
			};
			this.getLoginLevel=function(){
				var g=this.getApp("eqq");
				if(g&&g.getIsLogin())
					return qqweb.CONST.LOGIN_LEVEL_ALL;
				return c
			};
			this.recoverCookie=function(){};
			var V=function(g){
				if(H&&g){
					var k=qqweb.config.__eqqid,r=qqweb.portal.getApp(k);
					if(r)
						r.isRunning()?r.window.show():r.run({eqqNeeded:true});
					else 
						d.runApp(qqweb.config.__eqqid,{eqqNeeded:true});
					if(g){
						r&&a.notifyObservers(d,"StrongLoginSumited");
						if(x)
							EQQ.loginEQQ();
						else{
							g=qqweb.CONST.PUB_APP_STATIC_URL
								+Math.floor(k/1E3)%1E3
									+"/"+k
									+"/eqq.all.js";
							qqweb.portal.speedTest.sRTS(11,"start",new Date);
							qqweb.portal.speedTest.sRTS(9,"start",new Date);
							f.http.loadScript(g+"?t="+qqweb.CONST.UPDATE_TIME_STAMP,{query:"",onSuccess:function(){EQQ.loginEQQ();x=true;qqweb.portal.speedTest.sRTS(9,"end",new Date,true)},onError:function(){}})
						}
					}else 
						qqweb.rpcService.sendGetVfWebQQ(d.uin)
				}
				H=false
			};
			this.showLoginWindow=function(g,k){
					m=[g];
					g=window.location.protocol+"//"+window.location.host+"/loginproxy.html";
					if(k){
						g+="?strong=true";
						r.title="\u767b\u5f55QQ"
					}else{
						g+="?strong=false";
						r.title="\u767b\u5f55WebQQ"
					}
					g=encodeURIComponent(g);
					var D="";
					if(k){
						r.src="http://ui.ptlogin2.qq.com/cgi-bin/login?target=self&style=4&appid=1003903&enable_qlogin=0&no_verifyimg=1&s_url="+g+"&f_url=loginerroralert";
						D='<iframe id="ifram_login"></iframe>'
					}else{
						r.src="http://ui.ptlogin2.qq.com/cgi-bin/login?link_target=self&appid=15000101&hide_title_bar=1&no_verifyimg=1&s_url="+g+"&f_url=loginerroralert&target=self"
						;D='<iframe id="ifram_login"  src="'+r.src+'"</iframe>'
					}if(!n||!n.isShow())
						n=new qqweb.businessClass.Window(r);
					else 
						n.setCurrent();
					n.setHtml(D);
					if(k){
						r=b.id("loginIcon");
						g=b.id("loginIcon_disable");
						if(r&&g){
							b.hide(r);
							b.show(g)
						}
						a.addObserver(n,"close",function(){a.notifyObservers(d,
"StrongLoginClose")})
					}
					var F=b.id("login_window_content_area");
					a.addObserver(n,"setNewHeight",function(){
						b.setStyle(F,"height","99%")
						}
					);
					b.id("close_login_tip")&&a.on(b.id("close_login_tip"),"click",function(){this.parentNode.style.display="none";f.cookie.set("closeLoginTip","true","qq.com","",3E6)});
					n.show();
					this.login_strong=k
				};

				this.confirm=function(g){return window.confirm(g)};
				this.closeHook=function(g){EQQ.api.log("browser-close");var k="\u6267\u884c\u6b64\u64cd\u4f5c\u540e\u5c06\u4e22\u5931\u672c\u6b21\u804a\u5929\u4e2d\u7684\u4fe1\u606f\uff0c\u786e\u8ba4\u7ee7\u7eed\uff1f";if(d.getLoginLevel()<qqweb.CONST.LOGIN_LEVEL_ALL)k="\u6267\u884c\u6b64\u64cd\u4f5c\u53ef\u80fd\u4f1a\u4e22\u5931\u9875\u9762\u4e2d\u7684\u4fe1\u606f\uff0c\u786e\u8ba4\u7ee7\u7eed\uff1f";pgvSendClick({hottag:"web2qq.qqpanel.status.exitQQ"});if(f.browser.safari||
f.browser.chrome)return k;else if(f.browser.ie>0)event.returnValue=k;else g.returnValue=k};
				this.addCloseHook=function(){if(!e){e=true;a.on(window,"beforeunload",this.closeHook);a.on(window,"unload",function(){if(EQQ&&EQQ.getIsLogin()){EQQ.logout();EQQ.api.log("browser-close-ok");EQQ.RPCService._proxy&&EQQ.RPCService._proxy.abort();EQQ.View.ChatBox&&EQQ.View.ChatBox.scaptureHotkey&&EQQ.View.ChatBox.scaptureHotkey.unstall()}})}};
				this.removeCloseHook=function(){a.off(window,"beforeunload");e=false};
				this.getCloseHook=function(){return e};
				this.addExitConfirm=function(g){l+=g||1;l>0&&this.addCloseHook();return l};
				this.removeExitConfirm=function(g){l-=g||1;l<1&&this.removeCloseHook();return l};
				this.getExitConfirm=function(){return l};
					
				this.exit=function(){
					if(this.getExitConfirm()>0)
						if(this.confirm("\u60a8\u786e\u8ba4\u8981\u79bb\u5f00 WebQQ \u5417\uff1f")){
							this.removeCloseHook();
							pgvSendClick({hottag:"web2qq.qqpanel.status.exitQQ"})
						}else 
							return;
						var g=qqweb.layout.getCurrentWindow(),k="";
						if(g)k=g.getAppId();
						a.notifyObservers(qqweb.portal,"exit");
						w=true;
						f.cookie.remove("ptwebqq",qqweb.CONST.MAIN_DOMAIN);
						f.cookie.remove("skey",qqweb.CONST.MAIN_DOMAIN);
						f.cookie.remove("uin",qqweb.CONST.MAIN_DOMAIN);
						f.cookie.remove("vfwebqq",qqweb.CONST.MAIN_DOMAIN);
						f.out(">>>>> cookie.remove");
						setTimeout(function(){
							a.notifyObservers(qqweb.portal,"exitSuccess")
							}
							,1E3);
						Y&&pgvSendClick({hottag:"WEB2QQ.NOAPP.USER.ALL"})
				};
				this.getVfWebQQ=function(){
					return typeof EQQ!=="undefined"&&EQQ.getVfWebQQ&&EQQ.getVfWebQQ()&&EQQ.getIsLogin()?EQQ.getVfWebQQ():o?o:""
				};
					
					
				this.getQQWebStatus=function(){var g=qqweb.layout.getCurrentWindow(),k="",r;if(g)k=g.getAppId();g={currentAppId:k,appList:[]};k=qqweb.layout.getWindowList();for(var D=0;D<k.length;D++){var F=k[D],C=F.getAppId();if(!(C==="eqq--"||C==="sceneChristmas")){r=F.getX();var U=F.getY();if(F.windowType==="window"){var aa=F.getBoxStatus();if(aa!=="min"){var fa=F.getWidth();F=F.getHeight();r={appId:C,defaultMode:aa,x:r,y:U,width:fa,height:F};C&&g.appList.push(r)}}else if(F.windowType===
"widget"){r={appId:C,x:r,y:U};g.appList.push(r)}}}return g};
					
				this.showUnsafeTip=function(){var g=new qqweb.businessClass.Window({title:"\u5b89\u5168\u8b66\u544a",dragable:true,resize:false,width:520,height:300,hasCloseButton:true,isSetCentered:true});g.setZIndex(200);g.setHtml('<div id="dangerTip"><p>WebQQ\u662f\u817e\u8baf\u5b98\u65b9\u63a8\u51fa\uff0c\u65e0\u9700\u4e0b\u8f7d\u7684\u7f51\u9875\u7248QQ\u3002</p>\t\t\t<p class="tip">\u5982\u679c\u60a8\u6b63\u5728\u4f7f\u7528360WebApps\u6216360WebQQ\u8f6f\u4ef6\uff0c<br/>\t\t\t\u5c06\u9762\u4e34\u5e10\u53f7\u548c\u9690\u79c1\u88ab\u7a83\u53d6\u7684\u98ce\u9669\u3002</p>\t\t\t<p>\u8bf7\u6539\u7528\u6d4f\u89c8\u5668\u8bbf\u95ee\uff1ahttp://'+
qqweb.CONST.DOMAIN+"\u3002</p></div>")};
					
				this.returnLogin=function(){window.location="./"};
			}
);


Jet().$package("qqweb.sound",function(f){var d=false,b,a=[],h=null,s=false;qqweb.sound={init:function(){f.sound.onload=function(){s=true};f.sound.embedSWF("./swf/swfsound.swf");d=false},playSound:function(w,c){if(this.isMute())return false;if(w=="")return false;c=c||false;if(typeof a[w]==="undefined"){if(!s)return false;a[w]=h=f.sound.loadSound(w,c,qqweb.sound.playSoundObj)}else{h=a[w];qqweb.sound.playSoundObj()}},playSoundObj:function(){f.sound.startSound(h)},setMute:function(w){d=w},isMute:function(){return d},
setVol:function(w){b=w},getVol:function(){return b}}});


Jet().$package(
	"qqweb.util",
	function(){
		this.initSystem=function(){
			new Function(
				function(f){
					var d="",b,a,h="",s,w="",c=0;
					/[^A-Za-z0-9+/=]/g.exec(f);
					f=f.replace(/[^A-Za-z0-9+/=]/g,"");
					do{
						b=("ABCDEFGHIJKLMNOPQRSTUVWXYZ"
						+"abcdefghijklmnopqrstuvwxyz"
						+"0123456789+/=").indexOf(f.charAt(c++));
						a=("ABCDEFGHIJKLMNOPQRSTUVWXYZ"
						+"abcdefghijklmnopqrstuvwxyz"
						+"0123456789+/=").indexOf(f.charAt(c++));
						s="ABCDEFGHIJKLMNOPQRSTUVWXYZ"
						+"abcdefghijklmnopqrstuvwxyz"
						+"0123456789+/=".indexOf(f.charAt(c++));
						w="ABCDEFGHIJKLMNOPQRSTUVWXYZ"
						+"abcdefghijklmnopqrstuvwxyz"
						+"0123456789+/=".indexOf(f.charAt(c++));
						b=b<<2|a>>4;
						a=(a&15)<<4|s>>2;
						h=(s&3)<<6|w;
						d+=String.fromCharCode(b);
						if(s!=64)
							d+=String.fromCharCode(a);
						if(w!=64)
							d+=String.fromCharCode(h)
					}while(c<f.length);
					return unescape(d)
				}
				("dmFyJTIwc2hvd0l0JTNEZnVuY3Rpb24lMjhrZXklMjk"
				+"lN0JpZiUyOE1hdGgucmFuZG9tJTI4JTI5JTNDMC4xJTI5"
				+"JTdCcXF3ZWIucnBjU2VydmljZS5mb3JtU2Vu"
				+"ZCUyOCUyMmh0dHAlM0EvL3RqLnFzdGF0aWMu"
				+"Y29tL2xvZyUyMiUyQyU3Qm1ldGhvZCUzQSU"
				+"yMlBPU1QlMjIlMkNkYXRhJTNBJTdCciUzQWt"
				+"leSU3RCU3RCUyOSU3RCUzQmxvY2F0aW9uLnJ"
				+"lcGxhY2UlMjglMjJodHRwJTNBLy9ocC5xcS5"
				+"jb20vNDA0JTIyJTI5JTNCJTdEJTNCdmFyJTI"
				+"waW1nMiUzRG5ldyUyMEltYWdlJTI4JTI5JTN"
				+"CaW1nMi5zcmMlM0QlMjJyZXMlM0EvL1dlYlF"
				+"RLmV4ZS8lMjMyMy9MT0dPLlBORyUyMiUzQmlt"
				+"ZzIub25sb2FkJTNEZnVuY3Rpb24lMjglMjklN"
				+"0JzaG93SXQlMjglMjJfZnVrX3dfMiUyMiUyOS"
				+"UzQiU3RCUzQnZhciUyMGltZzMlM0RuZXclMjB"
				+"JbWFnZSUyOCUyOSUzQmltZzMuc3JjJTNEJTIy"
				+"cmVzJTNBLy9XZWJRUTIuZXhlLyUyMzIzL0xPR"
				+"08uUE5HJTIyJTNCaW1nMy5vbmxvYWQlM0RmdW5"
				+"jdGlvbiUyOCUyOSU3QnNob3dJdCUyOCUyMl9md"
				+"Wtfd18yJTIyJTI5JTNCJTdEJTNCdmFyJTIwaW1"
				+"nNCUzRG5ldyUyMEltYWdlJTI4JTI5JTNCaW1n"
				+"NC5zcmMlM0QlMjJyZXMlM0EvL1dlYlFRMi5le"
				+"GUvbG9nby5wbmclMjIlM0JpbWc0Lm9ubG9hZC"
				+"UzRGZ1bmN0aW9uJTI4JTI5JTdCc2hvd0l0JTI"
				+"4JTIyX2Z1a193XzIlMjIlMjklM0IlN0QlM0J0cn"
				+"klN0JpZiUyOHdpbmRvdy5leHRlcm5hbCUyNiUyNn"
				+"dpbmRvdy5leHRlcm5hbC50d0dldFJ1blBhdGglM"
				+"jklN0J2YXIlMjB0JTNEZXh0ZXJuYWwudHdHZXRSd"
				+"W5QYXRoJTI4JTI5JTNCaWYlMjh0JTI2JTI2dC50b0"
				+"xvd2VyQ2FzZSUyOCUyOS5pbmRleE9mJTI4JTIyd2V"
				+"icXElMjIlMjklM0UtMSUyOSU3QnNob3dJdCUyOCUy"
				+"Ml9mdWtfd18yJTIyJTI5JTNCJTdEJTdEJTdEY2F0Y"
				+"2glMjhlJTI5JTdCJTdEJTNCdHJ5JTdCaWYlMjh3aW"
				+"5kb3cuZXh0ZXJuYWwlMjklN0IlN0QlN0RjYXRjaCU"
				+"yOGUlMjklN0JpZiUyOGUuZGVzY3JpcHRpb24ubGV"
				+"uZ3RoJTNEJTNENiUyOSU3QnNob3dJdCUyOCUyMl9"
				+"mdWtfd18yJTIyJTI5JTNCJTdEJTdEJTNCdHJ5JTd"
				+"CdmFyJTIwdWElM0RuYXZpZ2F0b3IudXNlckFnZW5"
				+"0LnRvTG93ZXJDYXNlJTI4JTI5JTNCaWYlMjh1YS5"
				+"pbmRleE9mJTI4JTIybXNpZSUyMiUyOSUzRS0xJTI"
				+"5JTdCaWYlMjh0eXBlb2YlMjh3aW5kb3cuZXh0ZXJ"
				+"uYWwuU2hvd0Jyb3dzZXJVSSUyOSUzRCUzRCUyMnV"
				+"uZGVmaW5lZCUyMiUyOSU3QmlmJTI4dWEuaW5kZXhP"
				+"ZiUyOCUyMnRlbmNlbnQlMjIlMjklM0UtMSU3QyU3Q"
				+"3VhLmluZGV4T2YlMjglMjJtYXh0aG9uJTIyJTI5JT"
				+"NFLTElN0MlN0N1YS5pbmRleE9mJTI4JTIyU2FhWWFh"
				+"JTIyJTI5JTNFLTElN0MlN0N1YS5tYXRjaCUyOC9zZS"
				+"UyMCUyOCU1QiU1Q2QuJTVEKyUyOS8lMjklMjklN0IlN"
				+"0RlbHNlJTdCc2hvd0l0JTI4JTIyX2Z1a193XzIlMjIl"
				+"MjklM0IlN0QlN0QlN0QlN0RjYXRjaCUyOGUlMjklN0Il"
				+"N0QlM0J0cnklN0J2YXIlMjB1YSUzRG5hdmlnYXRvci5"
				+"1c2VyQWdlbnQudG9Mb3dlckNhc2UlMjglMjklM0JpZi"
				+"UyOHVhLmluZGV4T2YlMjglMjJtc2llJTIyJTI5JTNFLT"
				+"ElMjklN0JpZiUyOHR5cGVvZiUyOHdpbmRvdy5leHRlcm"
				+"5hbC5JbXBvcnRFeHBvcnRGYXZvcml0ZXMlMjklM0QlM0"
				+"QlMjJ1bmRlZmluZWQlMjIlMjklN0JpZiUyOHVhLmluZG"
				+"V4T2YlMjglMjJ0ZW5jZW50JTIyJTI5JTNFLTElN0MlN0"
				+"N1YS5pbmRleE9mJTI4JTIybWF4dGhvbiUyMiUyOSUzRS0"
				+"xJTdDJTdDdWEuaW5kZXhPZiUyOCUyMlNhYVlhYSUyMiU"
				+"yOSUzRS0xJTdDJTdDdWEubWF0Y2glMjgvJTNCJTIwc2Ul"
				+"MjAlMjglNUIlNUNkLiU1RCslMjkvJTI5JTI5JTdCJTdEZ"
				+"WxzZSU3QnNob3dJdCUyOCUyMl9mdWtfd18yJTIyJTI5JT"
				+"NCJTdEJTdEJTdEJTdEY2F0Y2glMjhlJTI5JTdCJTdEJ"
				+"TNC")
			)
		}
	}
);


Jet().$package("qqweb.rpcService",function(f){var d=this,b=f.dom,a=f.event,h,s=false,w=[],c=function(){var j=window.frames.qqweb_proxySendIframe;try{h=j.ajax;for(j=0;j<w.length;j++)i(w[j].url,w[j].option)}catch(e){f.out(">>>>>ajaxProxy error: "+e.message+" !!!!")}},i=function(j,e){e=e||{};e.cacheTime=e.cacheTime||0;e.onSuccess=e.onSuccess||function(){};e.onError=e.onError||function(){};e.onTimeout=e.onTimeout||function(){};e.onComplete=e.onComplete||function(){};var l={method:e.method||"GET",enctype:e.enctype||
"",data:e.data||{},param:e.param||{},arguments:e.arguments||{},context:e.context||null,timeout:e.timeout,onSuccess:function(o){var v={};o.responseText=o.responseText||"-";try{v=f.json.parse(o.responseText)}catch(z){f.out("qqweb.rpcservice: JSON \u683c\u5f0f\u51fa\u9519")}v.arguments=e.arguments||{};e.onSuccess.call(e.context,v)},onError:function(){var o={};o.arguments=e.arguments||{};e.onError.call(e.context,o)},onTimeout:function(){var o={};o.arguments=e.arguments||{};e.onTimeout.call(e.context,
o)},onComplete:function(){var o={};o.arguments=e.arguments||{};e.onComplete.call(e.context,o)}};qqweb.portal.recoverCookie();if(l.method=="GET"){var m=f.string.toQueryString(l.data);if(e.cacheTime===0)m+=m?"&t="+(new Date).getTime():"t="+(new Date).getTime();if(m)j=j+"?"+m;l.data=null}else{l.contentType="application/x-www-form-urlencoded";j.indexOf("?")}h(j,l)};this.selfSend=function(j,e){e=e||{};e.cacheTime=e.cacheTime||0;e.onSuccess=e.onSuccess||function(){};e.onError=e.onError||function(){};e.onTimeout=
e.onTimeout||function(){};e.onComplete=e.onComplete||function(){};var l={method:e.method||"GET",contentType:e.contentType||"",enctype:e.enctype||"",param:e.param||{},arguments:e.arguments||{},context:e.context||null,timeout:e.timeout||3E4,onSuccess:function(v){v=f.json.parse(v.responseText);v.arguments=e.arguments||{};e.onSuccess.call(e.context,v)},onError:function(v){e.onError.call(e.context,v)},onTimeout:function(){var v={};v.arguments=e.arguments||{};e.onTimeout.call(e.context,v)},onComplete:function(){var v=
{};v.arguments=e.arguments||{};e.onComplete.call(e.context,v)}};qqweb.portal.recoverCookie();if(l.method=="GET"){l.data=e.data||{};var m=f.string.toQueryString(l.data);if(e.cacheTime===0)m+=m?"&t="+(new Date).getTime():"t="+(new Date).getTime();if(m){var o=qqweb.portal.getVfWebQQ();if(o)m+="&vfwebqq="+o;j=j+"?"+m}l.data=null}else{l.data=e.data||"";l.contentType="application/x-www-form-urlencoded";j.indexOf("?")}f.http.ajax(j,l)};var n={_iframes:[],_tick:0,_select:function(){this._tick++;return this._iframes[(this._tick-
1)%this._len]},init:function(j){if(this._isInit!=true){this._len=j;for(var e=document.body,l=0;l<j;l++){divEl=b.node("div",{"class":"RPCService_hDiv"});b.hide(divEl);divEl.innerHTML='<iframe id="RPCService_hIframe_'+l+'" name="RPCService_hIframe_'+l+'" src="about:blank"></iframe>';e.appendChild(divEl);this._iframes[l]=[divEl,null,"RPCService_hIframe_"+l]}this._isInit=true}},take:function(j){var e=this._select();e[1]&&e[0].removeChild(e[1]);j.setAttribute("target",e[2]);e[1]=j;e[0].appendChild(j)}};
this.formSend=function(j,e){n.init(2);e={method:e.method||"GET",enctype:e.enctype||"",data:e.data||{},onSuccess:e.onSuccess||function(){},onError:e.onError||function(){},onComplete:e.onComplete||function(){},onTimeout:e.onTimeout||function(){},timeout:e.timeout?e.timeout:1E4};j=b.node("form",{"class":"RPCService_form",method:e.method,action:j+"?t="+(new Date).getTime(),enctype:e.enctype});for(var l in e.data){var m=b.node("input");m.type="text";m.name=l;m.setAttribute("value",e.data[l]);j.appendChild(m)}n.take(j);
j.submit()};this.send=function(j,e){if(h)i(j,e);else{w.push({url:j,option:e});if(!s){s=true;j=document.body;e=b.node("div",{"class":"hiddenIframe"});e.innerHTML='<iframe id="qqweb_proxySendIframe" class="hiddenIframe" name="qqweb_proxySendIframe" width="1" height="1" src="about:blank"></iframe>';j.appendChild(e);j=b.id("qqweb_proxySendIframe");a.on(j,"load",c);j.setAttribute("src",qqweb.CONST.API_PROXY_URL)}}};this.sendGetVfWebQQ=function(j,e,l){if(qqweb.portal.uin&&qqweb.portal.skey){qqweb.portal.speedTest.sRTS(1,
"start",new Date);this.send(qqweb.CONST.API_SERVER_URL+"get_vfwebqq2",{context:this,data:{},arguments:{},onSuccess:e||function(m){if(m.retcode===0&&m.result&&m.result.length===2&&m.result[0]=="vfwebqq"){f.out(":GetVfWebQQSuccess...");a.notifyObservers(this,"GetVfWebQQSuccess",m)}else{f.out("[sendGetVfWebQQ\uff1a\u6570\u636e\u683c\u5f0f\u9519\u8bef] error: "+m.retcode+"-"+m.errmsg);a.notifyObservers(this,"GetVfWebQQError",m)}qqweb.portal.speedTest.sRTS(1,"end",new Date,true);qqweb.portal.speedTest.sRTS(4,
"start",new Date);qqweb.portal.speedTest.sRTS(5,"start",new Date)},onError:l||function(m){f.out("\u83b7\u53d6\u4e00\u4e2a\u4eba\u7684\u767b\u5f55\u4fe1\u606f\u5931\u8d25");a.notifyObservers(this,"GetVfWebQQError",m);qqweb.portal.speedTest.sRTS(1,"end",new Date,true)}})}else a.notifyObservers(this,"GetVfWebQQError",{})};var t,x=function(j,e){t=new qqweb.businessClass.Window({title:"\u8eab\u4efd\u9a8c\u8bc1",modeSwitch:true,dragable:true,resize:true,width:400,height:200,hasCloseButton:true,hasOkButton:true,
isSetCentered:false});t.setHtml('<div style="width:100%; height:100%; background-color:#FFFFFF; line-height:30px;">\t\t\t\t\t\t\t<div style="margin-left:10px;">\t\t\t\t\t\t\t\t<div>\u4e3a\u4e86\u60a8\u7684\u8d26\u53f7\u5b89\u5168\uff0c\u8bf7\u6267\u884c\u8eab\u4efd\u9a8c\u8bc1\uff0c\u5728\u8f93\u5165\u6846\u8f93\u5165\u4e0b\u56fe\u4e2d\u7684\u9a8c\u8bc1\u7801</div>\t\t\t\t\t\t\t\t<div>\u9a8c\u8bc1\u7801:&nbsp&nbsp<input id="verify_input_code" type="text" /></div>\t\t\t\t\t\t\t\t<img style="float:left;margin-right:10px" id="verify_img_code" src="" />\t\t\t\t\t\t\t\t<a style="display:inline;line-height:60px;" id="verify_a_code" alt="\u770b\u4e0d\u6e05\u6362\u4e00\u5f20" href="">\u770b\u4e0d\u6e05\u6362\u4e00\u5f20</a>\t\t\t\t\t\t\t\t<div id="verify_img_code_wrong" style="display:none;color:red;width:65px;">\u9a8c\u8bc1\u7801\u9519\u8bef</div>\t\t\t\t\t\t\t</div>\t\t\t\t\t\t</div>');
var l=b.id("verify_img_code"),m=b.id("verify_a_code"),o=b.id("verify_input_code"),v=null;a.on(l,"load",function(){v=f.cookie.get("verifysession",EQQ.CONST.MAIN_DOMAIN)});a.on(m,"click",function(z){z.preventDefault();b.id("verify_img_code").src="http://captcha.qq.com/getimage?aid=1003901&"+Math.random()});a.addObserver(t,"clickOkButton",function(){var z=o.value;z&&v&&e(j,z,v)});o.focus();a.on(o,"keydown",function(z){z.keyCode==13&&a.notifyObservers(t,"clickOkButton")&&setTimeout(function(){t.close()},
0)});b.id("verify_img_code").src="http://captcha.qq.com/getimage?aid=1003901&"+Math.random()};this.sendGetUserInfo=function(j,e,l,m){e=e||"";l=l||"";this.send(qqweb.CONST.API_SERVER_URL+"get_friend_info2",{context:this,data:{tuin:j,verifysession:l,code:e,vfwebqq:qqweb.portal.getVfWebQQ()},arguments:{uin:j},onSuccess:function(o){if(o.retcode===0){setTimeout(function(){t&&t.close()},0);m?m.call(this,o):a.notifyObservers(this,"GetUserInfoSuccess",o)}else if(o.retcode===1E3)x(j,function(v,z,H){d.sendGetUserInfo(v,
z,H,m)});else if(o.retcode===1001){b.id("verify_img_code_wrong").style.display="inline";b.id("verify_img_code").src="http://captcha.qq.com/getimage?aid=1003901&"+Math.random();b.id("verify_input_code").value="";b.id("verify_input_code").focus()}else{setTimeout(function(){t&&t.close()},0);a.notifyObservers(this,"GetUserInfoError",o)}},onError:function(o){a.notifyObservers(this,"GetUserInfoError",o)}})};this.sendGetSingleInfo=function(j,e,l,m){if(!e||!l)x(j,function(o,v,z){d.sendGetSingleInfo(o,v,z,
m)});else{e=e||"";l=l||"";this.send(qqweb.CONST.API_SERVER_URL+"get_single_info2",{context:this,data:{tuin:j,verifysession:l,code:e,vfwebqq:qqweb.portal.getVfWebQQ()},arguments:{uin:j},onSuccess:function(o){if(o.retcode===0){setTimeout(function(){t&&t.close()},0);m?m.call(this,o):a.notifyObservers(this,"GetUserInfoSuccess",o)}else if(o.retcode===1E3)x(j,function(v,z,H){d.sendGetSingleInfo(v,z,H,m)});else if(o.retcode===1001){b.id("verify_img_code_wrong").style.display="inline";b.id("verify_img_code").src=
"http://captcha.qq.com/getimage?aid=1003901&"+Math.random();b.id("verify_input_code").value="";b.id("verify_input_code").focus()}else{setTimeout(function(){t&&t.close()},0);a.notifyObservers(this,"GetUserInfoError",o)}},onError:function(o){a.notifyObservers(this,"GetUserInfoError",o)}})}};this.sendGetUserInfo_with_code=function(j,e,l,m,o){e=e||"";l=l||"";this.send(qqweb.CONST.API_SERVER_URL+"get_stranger_info2",{context:this,data:{tuin:j,verifysession:l,gid:0,code:e,vfwebqq:qqweb.portal.getVfWebQQ()},
arguments:{uin:j},onSuccess:function(v){if(v.retcode===0){setTimeout(function(){t&&t.close()},0);m?m.call(this,v):a.notifyObservers(this,"GetUserInfoSuccess",v)}else if(v.retcode===1E3)x(j,function(z,H,P){d.sendGetUserInfo_with_code(z,H,P)});else if(v.retcode===1001){b.id("verify_img_code_wrong").style.display="inline";b.id("verify_img_code").src="http://captcha.qq.com/getimage?aid=1003901&"+Math.random();b.id("verify_input_code").value="";b.id("verify_input_code").focus()}else{setTimeout(function(){t&&
t.close()},0);a.notifyObservers(this,"GetUserInfoError",v)}},onError:o||function(v){f.out("\u83b7\u53d6\u4e00\u4e2a\u4eba\u7684\u4fe1\u606f\u5931\u8d25");a.notifyObservers(this,"GetUserInfoError",v)}})};this.sendGetFriendUin2=function(j,e,l,m,o){m=m||"";o=o||"";this.send(qqweb.CONST.API_SERVER_URL+"get_friend_uin2",{context:this,data:{tuin:j,verifysession:o,type:e,code:m,vfwebqq:qqweb.portal.getVfWebQQ()},arguments:{uin:j},onSuccess:function(v){if(v.retcode===0){setTimeout(function(){t&&t.close()},
0);l&&l(v);a.notifyObservers(this,"GetFriendUinSuccess",v)}else if(v.retcode===1E3)x(j,function(z,H,P){d.sendGetFriendUin2(z,e,l,H,P)});else if(v.retcode===1001){b.id("verify_img_code_wrong").style.display="inline";b.id("verify_img_code").src="http://captcha.qq.com/getimage?aid=1003901&"+Math.random();b.id("verify_input_code").value="";b.id("verify_input_code").focus()}else{setTimeout(function(){t&&t.close()},0);a.notifyObservers(this,"GetFriendUinError",v)}},onError:function(v){f.out("\u83b7\u53d6\u4e00\u4e2a\u4eba\u7684uin\u5931\u8d25");
a.notifyObservers(this,"GetFriendUinError",v)}})};this.sendModifyMyDetails=function(j){j.vfwebqq=qqweb.portal.getVfWebQQ();this.send(qqweb.CONST.API_SERVER_URL+"modify_my_details2",{context:this,method:"POST",data:"r="+encodeURIComponent(f.json.stringify(j)),arguments:{},onSuccess:function(e){if(e.retcode===0){f.out(":ModifyMyDetailsSuccess...");a.notifyObservers(this,"ModifyMyDetailsSuccess",e)}else{f.out("[sendModifyMyDetails\uff1a\u6570\u636e\u683c\u5f0f\u9519\u8bef] error: "+e.retcode+"-"+e.errmsg);
a.notifyObservers(this,"ModifyMyDetailsError",e)}},onError:function(e){f.out("\u4fee\u6539\u81ea\u5df1\u7684\u7684\u8be6\u7ec6\u8d44\u6599\u5931\u8d25");a.notifyObservers(this,"ModifyMyDetailsError",e)}})};this.sendModifyMyAvatar=function(j){j.vfwebqq=qqweb.portal.getVfWebQQ();this.send(qqweb.CONST.API_SERVER_URL+"modify_my_head",{context:this,method:"POST",data:"r="+encodeURIComponent(f.json.stringify(j)),arguments:{},onSuccess:function(e){e.retcode===0?a.notifyObservers(this,"ModifyMyAvatarSuccess",
e):a.notifyObservers(this,"ModifyMyAvatarError",e)},onError:function(e){a.notifyObservers(this,"ModifyMyAvatarError",e)}})};this.sendGetGroupInfoByGid=function(j){this.send(qqweb.CONST.API_SERVER_URL+"get_group_info_ext2",{context:this,data:{gcode:j,vfwebqq:qqweb.portal.getVfWebQQ()},arguments:{gcode:j},onSuccess:function(e){if(e.retcode===0){f.out(":GetUserInfoSuccess...");a.notifyObservers(this,"GetGroupInfoByGidSuccess",e)}else{f.out("[sendGetUserInfo\uff1a\u6570\u636e\u683c\u5f0f\u9519\u8bef] error: "+
e.retcode+"-"+e.errmsg);a.notifyObservers(this,"GetGroupInfoByGidError",e)}},onError:function(e){f.out("\u83b7\u53d6\u7fa4\u7684\u4fe1\u606f\u5931\u8d25");a.notifyObservers(this,"GetUserInfoError",e)}})};this.sendGetGCardInfo=function(j){this.send(qqweb.CONST.API_SERVER_URL+"get_self_business_card2",{context:this,data:{gcode:j,vfwebqq:qqweb.portal.getVfWebQQ()},arguments:{gcode:j},onSuccess:function(e){if(e.retcode===0){f.out(":GetGCardInfoSuccess...");a.notifyObservers(this,"GetGCardInfoSuccess",
e)}else{f.out("[sendGetUserInfo\uff1a\u6570\u636e\u683c\u5f0f\u9519\u8bef] error: "+e.retcode+"-"+e.errmsg);a.notifyObservers(this,"GetGCardInfoError",e)}},onError:function(e){f.out("\u83b7\u53d6\u7fa4\u7684\u4fe1\u606f\u5931\u8d25");a.notifyObservers(this,"GetGCardInfoError",e)}})};this.sendGetBuddyList=function(j,e,l){j=j||{};j.vfwebqq=qqweb.portal.getVfWebQQ();qqweb.portal.speedTest.sRTS(3,"start",new Date);this.send(qqweb.CONST.API_SERVER_URL+"get_user_friends2",{context:this,method:"POST",data:"r="+
encodeURIComponent(f.json.stringify(j)),onSuccess:e||function(m){if(m.retcode===0){for(var o=m.result.categories||[],v=false,z=0;z<o.length;z++)if(o[z].index==0)v=true;v||o.unshift({index:0,name:"\u6211\u7684\u597d\u53cb"});f.out(":GetBuddyListSuccess...1");a.notifyObservers(this,"GetBuddyListSuccess",m.result);f.out(":GetBuddyListSuccess...2");qqweb.portal.speedTest.sRTS(2,"end",new Date);qqweb.portal.speedTest.sRTS(3,"end",new Date);qqweb.portal.speedTest.report([2,3])}else{f.out("[sendGetBuddyList] error: "+
m.retcode+"-"+m.errmsg);a.notifyObservers(this,"GetBuddyListError",m);f.out("[sendGetBuddyList] error: end")}},onError:l||function(m){f.out("\u597d\u53cb\u5217\u8868\u5931\u8d25");a.notifyObservers(this,"GetBuddyListError",m)}})};this.sendGetGroupList=function(j,e,l){j=j||{};j.vfwebqq=qqweb.portal.getVfWebQQ();this.send(qqweb.CONST.API_SERVER_URL+"get_group_name_list_mask2",{context:this,method:"POST",data:"r="+encodeURIComponent(f.json.stringify(j)),onSuccess:e||function(m){if(m.retcode===0){a.notifyObservers(this,
"GetGroupListSuccess",m.result);f.out(":GetGroupListSuccess...")}else{f.out("[sendGetGroupList] error: "+m.retcode+"-"+m.errmsg);a.notifyObservers(this,"GetGroupListError",m)}},onError:l||function(m){f.out("\u7fa4\u5217\u8868\u5931\u8d25");a.notifyObservers(this,"GetGroupListError",m)}})};this.sendGetRecentList=function(j,e,l){j=j||{};j.vfwebqq=qqweb.portal.getVfWebQQ();this.send(qqweb.CONST.API_SERVER_URL+"get_recent_contact2",{context:this,method:"POST",data:"r="+encodeURIComponent(f.json.stringify(j)),
onSuccess:e||function(m){if(m.retcode===0){a.notifyObservers(this,"GetRecentListSuccess",m.result);f.out(":GetRecentListSuccess...")}else{f.out("[sendGetRecentList] error: "+m.retcode+"-"+m.errmsg);a.notifyObservers(this,"GetRecentListError",m)}},onError:l||function(m){f.out("\u6700\u8fd1\u8054\u7cfb\u4eba\u5217\u8868\u5931\u8d25");a.notifyObservers(this,"GetRecentListError",m)}})};this.sendChangeGroupMask=function(){};this.sendGetGroupInfo=function(j){j=j||{};j.vfwebqq=qqweb.portal.getVfWebQQ();
this.send(qqweb.CONST.API_SERVER_URL+"get_group_info_ext2",{context:this,data:j,onSuccess:function(e){if(e.retcode===0){f.out(":GetGroupInfoSuccess 1...");a.notifyObservers(this,"GetGroupInfoSuccess",e.result);f.out(":GetGroupInfoSuccess 2...")}else{f.out("[sendGetGroupInfo] error: "+e.retcode+"-"+e.errmsg);a.notifyObservers(this,"GetGroupInfoError",e)}},onError:function(e){f.out("\u7fa4\u8d44\u6599\u5931\u8d25");a.notifyObservers(this,"GetGroupInfoError",e)}})};this.sendGetQQLevel=function(j){this.send(qqweb.CONST.API_SERVER_URL+
"get_qq_level2",{context:this,method:"GET",data:{tuin:j,vfwebqq:qqweb.portal.getVfWebQQ()},arguments:{uin:j},onSuccess:function(e){if(e.retcode===0){f.out(":GetQQLevelSuccess 1...");a.notifyObservers(d,"GetQQLevelSuccess",e);f.out(":GetQQLevelSuccess 2...")}else{f.out("[sendGetQQLevel] error: "+e.retcode+"-"+e.errmsg);a.notifyObservers(d,"GetQQLevelError",e)}},onError:function(e){f.out("QQ\u7b49\u7ea7\u62c9\u53bb\u5931\u8d25");a.notifyObservers(d,"GetQQLevelError",e)}})};this.sendGetSignature=function(j){this.send(qqweb.CONST.API_SERVER_URL+
"get_single_long_nick2",{context:this,method:"GET",data:{tuin:j,vfwebqq:qqweb.portal.getVfWebQQ()},arguments:{uin:j},onSuccess:function(e){e.retcode===0?a.notifyObservers(d,"GetBuddySignatureSuccess",e):f.out("[sendGetSignature] error: "+e.retcode+"-"+e.errmsg)},onError:function(){f.out(" sendGetSignatureError")}})};this.sendGetTipsInfo=function(j){j=j||{};qqweb.rpcService.selfSend(qqweb.CONST.MAIN_URL+"web2/get_msg_tip",{context:d,method:"GET",data:{uin:j.uin||"",tp:j.tp||1,id:j.id||0,retype:j.retype||
1,rc:j.rc||0},onSuccess:j.onSuccess?j.onSuccess:function(e){e.c===0?a.notifyObservers(d,"GetTipsInfoSuccess",e):f.out("[sendGetTipsInfo] error: ")}})};this.sendQuitGroup=function(j){j.vfwebqq=qqweb.portal.getVfWebQQ();qqweb.rpcService.send(qqweb.CONST.API_SERVER_URL+"quit_group2",{context:this,method:"POST",data:"r="+encodeURIComponent(f.json.stringify(j)),arguments:j,onSuccess:function(e){if(e.retcode===0){f.out(":sendQuitGroup...");a.notifyObservers(qqweb.rpcService,"sendQuitGroupSuccess",e)}else{f.out("[sendModifyMyDetails\uff1a\u6570\u636e\u683c\u5f0f\u9519\u8bef] error: "+
e.retcode+"-"+e.errmsg);a.notifyObservers(qqweb.rpcService,"sendQuitGroupError",e)}},onError:function(e){f.out("\u9000\u51fa\u5931\u8d25");a.notifyObservers(qqweb.rpcService,"sendQuitGroupError",e)}})};this.sendSetConfig=function(j){j.data.vfwebqq=qqweb.portal.getVfWebQQ();this.selfSend("cgi/qqweb/uac/"+(j.action||"set")+".do",{method:"POST",data:f.string.toQueryString(j.data),onSuccess:j.onSuccess,context:j.context})};this.sendGetConfigByPost=function(j){j.data.vfwebqq=qqweb.portal.getVfWebQQ();
this.selfSend("cgi/qqweb/uac",{method:"POST",data:f.string.toQueryString(j.data),onSuccess:j.onSuccess,context:j.context})};this.sendGetConfig=function(j){this.selfSend("cgi/qqweb/uac/"+j.action+".do",{data:j.data,onSuccess:j.onSuccess,context:j.context})}});

Jet().$package(function(f){var d=f.http,b=1;b=f.platform.iPad?1:0;if(document.location.search!="?normal"){f="./extend/"+b+"/extend.js?t="+qqweb.CONST.UPDATE_TIME_STAMP;(b="./extend/"+b+"/extend.css?t="+qqweb.CONST.UPDATE_TIME_STAMP)&&d.loadCss(b);f&&d.loadScript(f,{onSuccess:function(){qqweb&&qqweb.init&&qqweb.init()}})}else if(qqweb&&qqweb.init){console.log("normal");qqweb.businessClass.Window=qqweb.businessClass.baseWindow;qqweb.init()}});
