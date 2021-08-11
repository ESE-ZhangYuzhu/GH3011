package com.goodix.project.weixin.device.controller;

import com.goodix.common.utils.RandomString;
import com.goodix.framework.config.WxMpConfiguration;
import com.goodix.framework.web.domain.JsTicketResult;
import me.chanjar.weixin.mp.api.WxMpService;
import org.apache.commons.codec.digest.DigestUtils;
import org.springframework.web.bind.annotation.*;

import java.util.Calendar;
import java.util.Date;
import java.util.Random;

/**
 * 操作状态
 *
 * @author goodix
 */
@RestController
@RequestMapping("/weixin/jsticket/{appid}")
public class JsTicketController {
    @GetMapping("")
    @ResponseBody
    public JsTicketResult getJsTicket(@PathVariable String appid, @RequestParam(name = "url", required = false) String url){
        JsTicketResult jsTicketResult=new JsTicketResult();
        final WxMpService wxService = WxMpConfiguration.getMpServices().get(appid);
        try {
            String jsTicket = wxService.getJsapiTicket(true);
            Integer timestamp = Integer.valueOf(String.valueOf((new Date()).getTime()/1000));
            RandomString rs = new RandomString();
            String noncestr = rs.generateString(new Random(),"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890",8);
            String originStr = String.format("jsapi_ticket=%s&noncestr=%s&timestamp=%s&url=%s",jsTicket,noncestr,timestamp,url);
            String signature=DigestUtils.sha1Hex(originStr);
            jsTicketResult.setJsTicket(jsTicket);
            jsTicketResult.setNoncestr(noncestr);
            jsTicketResult.setTimestamp(timestamp);
            jsTicketResult.setSignature(signature);
        } catch (Exception e) {
            e.printStackTrace();
        }
        return  jsTicketResult;
    }
}
