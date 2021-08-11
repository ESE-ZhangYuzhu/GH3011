package com.goodix.project.weixin.device.controller;

import com.goodix.common.constant.MpEventConstants;
import com.goodix.common.utils.RandomString;
import com.goodix.framework.config.WxMpConfiguration;
import com.goodix.framework.web.domain.JsTicketResult;
import com.goodix.project.weixin.demo.BlueLight;
import com.goodix.project.weixin.device.domain.BindInfo;
import com.goodix.project.weixin.device.domain.DeviceInfo;
import com.goodix.project.weixin.device.service.IBindInfoService;
import me.chanjar.weixin.common.api.WxConsts;
import me.chanjar.weixin.common.error.WxErrorException;
import me.chanjar.weixin.mp.api.WxMpService;
import me.chanjar.weixin.mp.bean.device.WxDeviceMsg;
import me.chanjar.weixin.mp.bean.kefu.WxMpKefuMessage;
import me.chanjar.weixin.mp.bean.message.WxMpJsonMessage;
import me.chanjar.weixin.mp.bean.message.WxMpXmlMessage;
import me.chanjar.weixin.mp.bean.message.WxMpXmlOutMessage;
import me.chanjar.weixin.mp.util.crypto.WxMpCryptUtil;
import org.apache.commons.codec.binary.Base64;
import org.apache.commons.codec.digest.DigestUtils;
import org.apache.commons.lang3.StringUtils;
import org.apache.shiro.crypto.hash.Sha1Hash;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.*;

import java.util.Date;
import java.util.List;
import java.util.Random;


/**
 * @ProjectName: weixin
 * @Package: com.goodix.project.weixin.device.controller
 * @ClassName: WeiXinDeviceController
 * @Description: 微信硬件事件路由(只适用与处理微信硬件的事件)
 * @Author: liaoxiaohua
 * @Version: 1.0
 */
@RestController
@RequestMapping("/weixin/device/{appid}")
public class WeiXinDeviceController {
    private final Logger logger = LoggerFactory.getLogger(this.getClass());

    @Autowired
    private IBindInfoService bindInfoService;

    @GetMapping(produces = "text/plain;charset=utf-8")
    public String authGet(@PathVariable String appid,
                          @RequestParam(name = "signature", required = false) String signature,
                          @RequestParam(name = "timestamp", required = false) String timestamp,
                          @RequestParam(name = "nonce", required = false) String nonce,
                          @RequestParam(name = "echostr", required = false) String echostr) {
        this.logger.info("\n接收到来自微信硬件服务器的认证消息：[{}, {}, {}, {}]", signature,
                timestamp, nonce, echostr);
        if (StringUtils.isAnyBlank(signature, timestamp, nonce, echostr)) {
            throw new IllegalArgumentException("请求参数非法，请核实!");
        }

        final WxMpService wxService = WxMpConfiguration.getMpServices().get(appid);
        if (wxService == null) {
            throw new IllegalArgumentException(String.format("未找到对应appid=[%d]的配置，请核实！", appid));
        }

        if (wxService.checkSignature(timestamp, nonce, signature)) {
            return echostr;
        }

        return "非法请求";
    }

    @PostMapping(produces = "application/xml; charset=UTF-8")
    public String post(@PathVariable String appid,
                       @RequestBody String requestBody,
                       @RequestParam(name="signature",required = false) String signature,
                       @RequestParam(name = "timestamp",required = false) String timestamp,
                       @RequestParam(name = "nonce",required = false) String nonce,
                       @RequestParam(name="openid",required = false) String openid,
                       @RequestParam(name = "encrypt_type", required = false) String encType,
                       @RequestParam(name = "msg_signature", required = false) String msgSignature) {
        final WxMpService wxService = WxMpConfiguration.getMpServices().get(appid);
        this.logger.info("\n接收微信硬件请求：[openid=[{}], [signature=[{}], encType=[{}], msgSignature=[{}],"
                        + " timestamp=[{}], nonce=[{}], requestBody=[\n{}\n] ",
                openid, signature, encType, msgSignature, timestamp, nonce, requestBody);

        if(StringUtils.isNotBlank(signature)){
            if (!wxService.checkSignature(timestamp, nonce, signature)) {
                throw new IllegalArgumentException("非法请求，可能属于伪造的请求！");
            }
        }
        String out = null;
        boolean isAesEncType=false;
        WxMpJsonMessage inMessage = new WxMpJsonMessage();

        if (encType == null) {
            // 明文传输的消息
            inMessage = WxMpJsonMessage.fromJson(requestBody);
            this.logger.debug("\n消息内容为：\n{} ", inMessage.toString());
        } else if ("aes".equalsIgnoreCase(encType)) {
            // aes加密的消息
            inMessage = WxMpJsonMessage.fromEncryptedJson(requestBody, wxService.getWxMpConfigStorage(),
                    timestamp, nonce, msgSignature);
            this.logger.debug("\n消息解密后内容为：\n{} ", inMessage.toString());
            isAesEncType=true;

        }
        out = routeDevice(wxService,inMessage);
        this.logger.debug("\n组装回复明文信息：{}", out);
        if(isAesEncType){
            WxMpCryptUtil pc = new WxMpCryptUtil(wxService.getWxMpConfigStorage());
            out=pc.encrypt(out);
            this.logger.debug("\n组装回复密文信息：{}", out);
            return out;
        }
        return out;
    }
    /**
     * 设备消息响应格式
     */
    public static final String DEVICE_TEXT = new StringBuilder()
            .append("<xml>")
            .append("<ToUserName><![CDATA[%s]]></ToUserName>")
            .append("<FromUserName><![CDATA[%s]]></FromUserName>")
            .append("<CreateTime>%s</CreateTime>")
            .append("<MsgType><![CDATA[%s]]></MsgType>")
            .append("<DeviceType><![CDATA[%s]]></DeviceType>")
            .append("<DeviceID><![CDATA[%s]]></DeviceID>")
            .append("<SessionID>%s</SessionID>")
            .append("<Content><![CDATA[%s]]></Content>")
            .append("</xml>").toString();

    /**
     * 构造设备消息响应
     */
    public static final String buildDeviceText(String fromUser, String toUser,
                                               String deviceType, String deviceID, String content, String sessionID) {
        return String.format(DEVICE_TEXT, fromUser, toUser, time(),
                WxConsts.XmlMsgType.DEVICE_TEXT, deviceType, deviceID, sessionID, content);
    }

    /**
     * 秒级时间戳
     */
    private static String time(){
        return String.valueOf(System.currentTimeMillis() / 1000);
    }

    private String routeDevice(WxMpService wxMpService,WxMpJsonMessage inMessage){
        String out="";
        this.logger.debug("event{}",inMessage.getEvent());
        this.logger.debug("eventKey{}",inMessage.getEventKey());
        if(MpEventConstants.Device.BIND.equals(inMessage.getMsgType()) || MpEventConstants.Device.UNBIND.equals(inMessage.getMsgType())){
            BindInfo bindInfo=new BindInfo();
            bindInfo.setOpenId(inMessage.getOpenId());
            bindInfo.setDeviceId(inMessage.getDeviceId());
            BindInfo bindInfoOld = bindInfoService.selectInfoByInfo(bindInfo);
            if(MpEventConstants.Device.BIND.equals(inMessage.getMsgType())){
                bindInfo.setStatus(1);
                bindInfo.setUpdateTime(new Date());
                this.logger.debug("设备绑定事件");
                WxMpKefuMessage kefuMessage=new WxMpKefuMessage();
                kefuMessage.setToUser(inMessage.getOpenId());
                kefuMessage.setContent("绑定成功");
                kefuMessage.setMsgType(WxConsts.KefuMsgType.TEXT);
                try {
                    wxMpService.getKefuService().sendKefuMessage(kefuMessage);
                } catch (WxErrorException e) {
                    e.printStackTrace();
                }
            }else{
                bindInfo.setStatus(0);
                bindInfo.setUpdateTime(new Date());
                this.logger.debug("设备解绑");
            }
            if(bindInfoOld==null)
                bindInfoService.insertInfo(bindInfo);
            else
                bindInfoService.updateInfo(bindInfo);
            out=buildDeviceText(inMessage.getFromUser(),inMessage.getToUser(),inMessage.getDeviceType(),inMessage.getDeviceId(),inMessage.getContent(),inMessage.getSessionID());
        }
        if(WxConsts.XmlMsgType.DEVICE_TEXT.equals(inMessage.getMsgType())){
            byte[] reqContent = Base64.decodeBase64(inMessage.getContent());
            String transText = new String(reqContent);
            this.logger.debug("接收到设备发送来的数据{}",transText);
            try {
                WxMpKefuMessage kefuMessage=new WxMpKefuMessage();
                kefuMessage.setToUser(inMessage.getOpenId());
                kefuMessage.setContent(transText);
                kefuMessage.setMsgType(WxConsts.KefuMsgType.TEXT);
                wxMpService.getKefuService().sendKefuMessage(kefuMessage);

                // 序列化
                byte[] respRaw = transText.getBytes();
                // Base64编码
                String respCon = Base64.encodeBase64String(respRaw);
                out=buildDeviceText(inMessage.getFromUser(),inMessage.getToUser(),inMessage.getDeviceType(),inMessage.getDeviceId(),respCon,inMessage.getSessionID());
                //wxService.getKefuService().sendKefuMessage()
            } catch (Exception e) {
                e.printStackTrace();
            }

        }

        return out;
    }

    private WxMpXmlOutMessage route(WxMpXmlMessage message, String appid) {
        try {
            return WxMpConfiguration.getRouters().get(appid).route(message);
        } catch (Exception e) {
            this.logger.error("路由消息时出现异常！", e);
        }

        return null;
    }
}
