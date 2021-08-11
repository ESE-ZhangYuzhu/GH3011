package com.goodix.project.weixin.device.controller;


import com.goodix.common.constant.MpEventConstants;
import com.goodix.framework.config.WxMpConfiguration;
import com.goodix.project.weixin.demo.BlueLight;
import com.goodix.project.weixin.device.domain.BindInfo;
import com.goodix.project.weixin.device.domain.DeviceInfo;
import com.goodix.project.weixin.device.service.IBindInfoService;
import me.chanjar.weixin.common.api.WxConsts;
import me.chanjar.weixin.common.error.WxErrorException;
import me.chanjar.weixin.mp.api.WxMpService;
import me.chanjar.weixin.mp.bean.kefu.WxMpKefuMessage;
import me.chanjar.weixin.mp.bean.message.WxMpXmlMessage;
import me.chanjar.weixin.mp.bean.message.WxMpXmlOutMessage;
import org.apache.commons.codec.binary.Base64;
import org.apache.commons.lang3.StringUtils;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.*;

import java.util.Date;
import java.util.List;

/**
 * @ProjectName: weixin
 * @Package: com.goodix.project.weixin.device.controller
 * @ClassName: WeiXinPortalController
 * @Description: 微信公众号信息路由(微信公众号与微信硬件共用)
 * @Author: liaoxiaohua
 * @Version: 1.0
 */
@RestController
@RequestMapping("/weixin/portal/{appid}")
public class WeiXinPortalController {

    private final Logger logger = LoggerFactory.getLogger(this.getClass());
    @Autowired
    private IBindInfoService bindInfoService;

    @GetMapping(produces = "text/plain;charset=utf-8")
    public String authGet(@PathVariable String appid,
                          @RequestParam(name = "signature", required = false) String signature,
                          @RequestParam(name = "timestamp", required = false) String timestamp,
                          @RequestParam(name = "nonce", required = false) String nonce,
                          @RequestParam(name = "echostr", required = false) String echostr) {

        this.logger.info("\n接收到来自微信服务器的认证消息：[{}, {}, {}, {}]", signature,
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
        this.logger.info("\n接收微信请求：[openid=[{}], [signature=[{}], encType=[{}], msgSignature=[{}],"
                + " timestamp=[{}], nonce=[{}], requestBody=[\n{}\n] ",
            openid, signature, encType, msgSignature, timestamp, nonce, requestBody);

        if(StringUtils.isNotBlank(signature)) {
            if (!wxService.checkSignature(timestamp, nonce, signature)) {
                throw new IllegalArgumentException("非法请求，可能属于伪造的请求！");
            }
        }
        String out = null;
        if (encType == null) {
            // 明文传输的消息
            WxMpXmlMessage inMessage = WxMpXmlMessage.fromXml(requestBody);
            this.logger.debug("\n消息内容为：\n{} ", inMessage.toString());
            if(WxConsts.XmlMsgType.DEVICE_EVENT.equals(inMessage.getMsgType()) || WxConsts.XmlMsgType.DEVICE_TEXT.equals(inMessage.getMsgType())){
                this.logger.debug("设备事件");
                return routeDevice(wxService,inMessage);
            }else{
                if(inMessage.getMsgType().equals(WxConsts.XmlMsgType.EVENT) && inMessage.getEvent().equals(WxConsts.EventType.CLICK)){
                    BindInfo queryBind = new BindInfo();
                    if(StringUtils.isBlank(inMessage.getOpenId())){
                        queryBind.setOpenId(inMessage.getFromUser());
                        inMessage.setOpenId(inMessage.getFromUser());
                    }
                    else
                        queryBind.setOpenId(inMessage.getOpenId());
                    DeviceInfo queryDeviceInfo=new DeviceInfo();
                    queryDeviceInfo.setAppId(appid);
                    queryBind.setDeviceInfo(queryDeviceInfo);
                    List<BindInfo> bindInfoList=bindInfoService.selectInfoList(queryBind);
                    if(bindInfoList!=null && bindInfoList.size()>0){
                        inMessage.setDeviceId(bindInfoList.get(0).getDeviceId());
                        inMessage.setDeviceType(bindInfoList.get(0).getDeviceInfo().getDeviceType());
                        inMessage.setOpenId(inMessage.getOpenId());
                    }
                }
            }
            WxMpXmlOutMessage outMessage = this.route(inMessage, appid);
            if (outMessage == null) {
                return "";
            }
            out = outMessage.toXml();
        } else if ("aes".equalsIgnoreCase(encType)) {
            // aes加密的消息
            WxMpXmlMessage inMessage = WxMpXmlMessage.fromEncryptedXml(requestBody, wxService.getWxMpConfigStorage(),
                timestamp, nonce, msgSignature);
            this.logger.debug("\n消息解密后内容为：\n{} ", inMessage.toString());
            //如果是设备事件
            if(WxConsts.XmlMsgType.DEVICE_EVENT.equals(inMessage.getMsgType()) || WxConsts.XmlMsgType.DEVICE_TEXT.equals(inMessage.getMsgType())){
                this.logger.debug("设备事件");
                return routeDevice(wxService,inMessage);
            }

            WxMpXmlOutMessage outMessage = this.route(inMessage, appid);
            if (outMessage == null) {
                return "";
            }

            out = outMessage.toEncryptedXml(wxService.getWxMpConfigStorage());
        }

        this.logger.debug("\n组装回复信息：{}", out);
        return out;
    }
    private String routeDevice(WxMpService wxMpService,WxMpXmlMessage mpXmlMessage){
        String out="";
        this.logger.debug("event{}",mpXmlMessage.getEvent());
        this.logger.debug("eventKey{}",mpXmlMessage.getEventKey());
        if(MpEventConstants.Device.BIND.equals(mpXmlMessage.getEvent()) || MpEventConstants.Device.UNBIND.equals(mpXmlMessage.getEvent())){
            BindInfo bindInfo=new BindInfo();
            bindInfo.setOpenId(mpXmlMessage.getOpenId());
            bindInfo.setDeviceId(mpXmlMessage.getDeviceId());
            BindInfo bindInfoOld = bindInfoService.selectInfoByInfo(bindInfo);
            if(MpEventConstants.Device.BIND.equals(mpXmlMessage.getEvent())){
                bindInfo.setStatus(1);
                bindInfo.setUpdateTime(new Date());
                this.logger.debug("设备绑定事件");
                WxMpKefuMessage kefuMessage=new WxMpKefuMessage();
                kefuMessage.setToUser(mpXmlMessage.getOpenId());
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
            out=buildDeviceText(mpXmlMessage.getFromUser(),mpXmlMessage.getToUser(),mpXmlMessage.getDeviceType(),mpXmlMessage.getDeviceId(),mpXmlMessage.getContent(),mpXmlMessage.getSessionID());
        }
        if(WxConsts.XmlMsgType.DEVICE_TEXT.equals(mpXmlMessage.getMsgType())){
            byte[] reqContent = Base64.decodeBase64(mpXmlMessage.getContent());
            String transText = new String(reqContent);
            this.logger.debug("接收到设备发送来的数据{}",transText);
            try {
                WxMpKefuMessage kefuMessage=new WxMpKefuMessage();
                kefuMessage.setToUser(mpXmlMessage.getOpenId());
                kefuMessage.setContent(transText);
                kefuMessage.setMsgType(WxConsts.KefuMsgType.TEXT);
                wxMpService.getKefuService().sendKefuMessage(kefuMessage);

                // 序列化
                byte[] respRaw = transText.getBytes();
                // Base64编码
                String respCon = Base64.encodeBase64String(respRaw);
                out=buildDeviceText(mpXmlMessage.getFromUser(),mpXmlMessage.getToUser(),mpXmlMessage.getDeviceType(),mpXmlMessage.getDeviceId(),respCon,mpXmlMessage.getSessionID());
                //wxService.getKefuService().sendKefuMessage()
            } catch (Exception e) {
                e.printStackTrace();
            }

        }
        this.logger.debug("\n组装回复信息：{}", out);
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
    private static final String buildDeviceText(String fromUser, String toUser,
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

}
