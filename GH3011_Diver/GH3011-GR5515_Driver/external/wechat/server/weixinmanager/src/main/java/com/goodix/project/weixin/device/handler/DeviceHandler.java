package com.goodix.project.weixin.device.handler;

import me.chanjar.weixin.common.session.WxSessionManager;
import me.chanjar.weixin.mp.api.WxMpService;
import me.chanjar.weixin.mp.bean.message.WxMpXmlMessage;
import me.chanjar.weixin.mp.bean.message.WxMpXmlOutMessage;
import org.springframework.stereotype.Component;

import java.util.Map;

/**
 * 操作状态
 *
 * @author goodix
 */
@Component
public class DeviceHandler extends AbstractHandler  {
    @Override
    public WxMpXmlOutMessage handle(WxMpXmlMessage wxMessage,
                                    Map<String, Object> context, WxMpService wxMpService,
                                    WxSessionManager sessionManager) {
        //TODO 对设备信息进行做处理

        this.logger.info("新绑定设备 deviceId: " + wxMessage.getDeviceId());
        this.logger.info("新绑定设备 deviceType: " + wxMessage.getDeviceType());
        this.logger.info("新绑定设备用户OPENID: " + wxMessage.getFromUser());

        return null;
    }
}
