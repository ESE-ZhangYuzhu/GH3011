package com.goodix.framework.config;

import com.goodix.common.constant.MpEventConstants;
import com.goodix.project.weixin.device.handler.*;
import com.google.common.collect.Maps;
import me.chanjar.weixin.common.api.WxConsts;
import me.chanjar.weixin.mp.api.WxMpInMemoryConfigStorage;
import me.chanjar.weixin.mp.api.WxMpMessageRouter;
import me.chanjar.weixin.mp.api.WxMpService;
import me.chanjar.weixin.mp.api.impl.WxMpServiceImpl;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.context.properties.EnableConfigurationProperties;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;

import java.util.Map;
import java.util.stream.Collectors;

/**
 * ζδ½ηΆζ
 *
 * @author goodix
 */
@Configuration
@EnableConfigurationProperties(WxMpProperties.class)
public class WxMpConfiguration {
    private LogHandler logHandler;
    private NullHandler nullHandler;
    private KfSessionHandler kfSessionHandler;
    private StoreCheckNotifyHandler storeCheckNotifyHandler;
    private LocationHandler locationHandler;
    private MenuHandler menuHandler;
    private MsgHandler msgHandler;
    private UnsubscribeHandler unsubscribeHandler;
    private SubscribeHandler subscribeHandler;
    private DeviceHandler deviceHandler;

    private WxMpProperties properties;

    private static Map<String, WxMpMessageRouter> routers = Maps.newHashMap();
    private static Map<String, WxMpService> mpServices = Maps.newHashMap();

    @Autowired
    public WxMpConfiguration(LogHandler logHandler, NullHandler nullHandler, KfSessionHandler kfSessionHandler,
                             StoreCheckNotifyHandler storeCheckNotifyHandler, LocationHandler locationHandler,
                             MenuHandler menuHandler, MsgHandler msgHandler, UnsubscribeHandler unsubscribeHandler,
                             SubscribeHandler subscribeHandler,DeviceHandler deviceHandler, WxMpProperties properties) {
        this.logHandler = logHandler;
        this.nullHandler = nullHandler;
        this.kfSessionHandler = kfSessionHandler;
        this.storeCheckNotifyHandler = storeCheckNotifyHandler;
        this.locationHandler = locationHandler;
        this.menuHandler = menuHandler;
        this.msgHandler = msgHandler;
        this.unsubscribeHandler = unsubscribeHandler;
        this.subscribeHandler = subscribeHandler;
        this.deviceHandler = deviceHandler;
        this.properties = properties;
    }

    public static Map<String, WxMpMessageRouter> getRouters() {
        return routers;
    }

    public static Map<String, WxMpService> getMpServices() {
        return mpServices;
    }

    @Bean
    public Object services() {
        mpServices = this.properties.getConfigs()
                .stream()
                .map(a -> {
                    WxMpInMemoryConfigStorage configStorage = new WxMpInMemoryConfigStorage();
                    configStorage.setAppId(a.getAppId());
                    configStorage.setSecret(a.getSecret());
                    configStorage.setToken(a.getToken());
                    configStorage.setAesKey(a.getAesKey());

                    // WxMpService wxMpService = new me.chanjar.weixin.mp.api.impl.WxMpServiceOkHttpImpl();
                    // WxMpService wxMpService = new me.chanjar.weixin.mp.api.impl.WxMpServiceJoddHttpImpl();
                    // WxMpService wxMpService = new me.chanjar.weixin.mp.api.impl.WxMpServiceHttpClientImpl();
                    WxMpService service = new WxMpServiceImpl();
                    service.setWxMpConfigStorage(configStorage);
                    routers.put(a.getAppId(), this.newRouter(service));
                    return service;
                }).collect(Collectors.toMap(s -> s.getWxMpConfigStorage().getAppId(), a -> a));

        return Boolean.TRUE;
    }

    private WxMpMessageRouter newRouter(WxMpService wxMpService) {
        final WxMpMessageRouter newRouter = new WxMpMessageRouter(wxMpService);

        // θ?°ε½ζζδΊδ»Άηζ₯εΏ οΌεΌζ­₯ζ§θ‘οΌ
        newRouter.rule().handler(this.logHandler).next();

        //ζ₯ζΆθ?Ύε€δΊδ»Ά
        newRouter.rule().async(false).msgType(WxConsts.XmlMsgType.DEVICE_EVENT)
                .event(MpEventConstants.Device.BIND)
                .handler(this.deviceHandler).end();

        // ζ₯ζΆε?’ζδΌθ―η?‘ηδΊδ»Ά
        newRouter.rule().async(false).msgType(WxConsts.XmlMsgType.EVENT)
                .event(MpEventConstants.CustomerService.KF_CREATE_SESSION)
                .handler(this.kfSessionHandler).end();
        newRouter.rule().async(false).msgType(WxConsts.XmlMsgType.EVENT)
                .event(MpEventConstants.CustomerService.KF_CLOSE_SESSION)
                .handler(this.kfSessionHandler)
                .end();
        newRouter.rule().async(false).msgType(WxConsts.XmlMsgType.EVENT)
                .event(MpEventConstants.CustomerService.KF_SWITCH_SESSION)
                .handler(this.kfSessionHandler).end();

        // ι¨εΊε?‘ζ ΈδΊδ»Ά
        newRouter.rule().async(false).msgType(WxConsts.XmlMsgType.EVENT)
                .event(MpEventConstants.POI_CHECK_NOTIFY)
                .handler(this.storeCheckNotifyHandler).end();

        // θͺε?δΉθεδΊδ»Ά
        newRouter.rule().async(false).msgType(WxConsts.XmlMsgType.EVENT)
                .event(WxConsts.MenuButtonType.CLICK).handler(this.menuHandler).end();

        // ηΉε»θεθΏζ₯δΊδ»Ά
        newRouter.rule().async(false).msgType(WxConsts.XmlMsgType.EVENT)
                .event(WxConsts.MenuButtonType.VIEW).handler(this.nullHandler).end();

        // ε³ζ³¨δΊδ»Ά
        newRouter.rule().async(false).msgType(WxConsts.XmlMsgType.EVENT)
                .event(WxConsts.EventType.SUBSCRIBE).handler(this.subscribeHandler)
                .end();

        // εζΆε³ζ³¨δΊδ»Ά
        newRouter.rule().async(false).msgType(WxConsts.XmlMsgType.EVENT)
                .event(WxConsts.EventType.UNSUBSCRIBE)
                .handler(this.unsubscribeHandler).end();

        // δΈζ₯ε°ηδ½η½?δΊδ»Ά
        newRouter.rule().async(false).msgType(WxConsts.XmlMsgType.EVENT)
                .event(WxConsts.EventType.LOCATION).handler(this.locationHandler)
                .end();

        // ζ₯ζΆε°ηδ½η½?ζΆζ―
        newRouter.rule().async(false).msgType(WxConsts.XmlMsgType.LOCATION)
                .handler(this.locationHandler).end();

        // ζ«η δΊδ»Ά
        newRouter.rule().async(false).msgType(WxConsts.XmlMsgType.EVENT)
                .event(WxConsts.EventType.SCAN).handler(this.nullHandler).end();

        // ι»θ?€
        newRouter.rule().async(false).handler(this.msgHandler).end();

        return newRouter;
    }
}
