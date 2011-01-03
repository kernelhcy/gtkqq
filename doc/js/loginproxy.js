var searchStr=document.location.search;
var queryArr;
function getQuery(id){
	if(searchStr.length<=1)
		return "";
	if(searchStr.indexOf("<")!=-1 
			|| searchStr.indexOf("%3c")!=-1 
			|| searchStr.indexOf("%3C")!=-1)
		return "";//<
	if(!queryArr){
		queryArr={};
		searchStr=searchStr.substr(1);
		var arr=searchStr.split("&");
		for(var i=0;i<arr.length;++i){
			var tmp=arr[i].split("=");
			queryArr[tmp[0]]=decodeURI(tmp[1]);
		}
	}
	return queryArr[id];
}

function validateJumpUrl(url,domain){
	if(!domain)domain="www.qq.com";
	return (location.host.indexOf(domain)+domain.length==location.host.length);
}

document.domain="qq.com";
var strong=getQuery('strong')=='true';
try{
	parent.qqweb.portal.reRunApps(strong);
}catch(e){}
try{
	parent.qqweb.portal.hideLoginWindow();
}catch(e){}
