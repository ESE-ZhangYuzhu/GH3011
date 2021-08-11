package com.goodix.project.weixin.device.handler;

import me.chanjar.weixin.common.error.WxErrorException;
import me.chanjar.weixin.common.session.WxSessionManager;
import me.chanjar.weixin.mp.api.WxMpService;
import me.chanjar.weixin.mp.bean.device.WxDeviceMsg;
import me.chanjar.weixin.mp.bean.message.WxMpXmlMessage;
import me.chanjar.weixin.mp.bean.message.WxMpXmlOutMessage;
import org.apache.commons.codec.binary.Base64;
import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.util.Map;

import static me.chanjar.weixin.common.api.WxConsts.MenuButtonType;

/**
 * @author Binary Wang(https://github.com/binarywang)
 */
@Component
public class MenuHandler extends AbstractHandler {

    @Override
    public WxMpXmlOutMessage handle(WxMpXmlMessage wxMessage,
                                    Map<String, Object> context, WxMpService weixinService,
                                    WxSessionManager sessionManager) {
        WxDeviceMsg wxDeviceMsg=new WxDeviceMsg();
        String msg="";
        if(StringUtils.isNotBlank(wxMessage.getDeviceType())){
            wxDeviceMsg.setContent(Base64.encodeBase64String(wxMessage.getEventKey().getBytes()));
            wxDeviceMsg.setDeviceType(wxMessage.getDeviceType());
            wxDeviceMsg.setDeviceId(wxMessage.getDeviceId());
            wxDeviceMsg.setOpenId(wxMessage.getOpenId());
            try {
                this.logger.debug("发送数据到设备");
                msg = String.format("type:%s, event:%s, key:%s",
                        wxMessage.getMsgType(), wxMessage.getEvent(),
                        wxMessage.getEventKey());
                weixinService.getDeviceService().transMsg(wxDeviceMsg);
            } catch (WxErrorException e) {
                this.logger.debug("发送数据到设备失败{}",e.getMessage());
                e.printStackTrace();
            }
        }else {
            msg="没有设备";
        }
        if (MenuButtonType.VIEW.equals(wxMessage.getEvent())) {
            return null;
        }
        return WxMpXmlOutMessage.TEXT().content(msg)
            .fromUser(wxMessage.getToUser()).toUser(wxMessage.getFromUser())
            .build();
    }

}
