package com.goodix.project.weixin.device.domain;

import com.goodix.framework.web.domain.BaseEntity;
import org.apache.commons.lang3.builder.ToStringBuilder;
import org.apache.commons.lang3.builder.ToStringStyle;

/**
 * 硬件基础表 device_info
 *
 * @author goodix
 */
public class DeviceInfo extends BaseEntity
{
    private static final long serialVersionUID = 1L;

    /**  */
    private String deviceId;
    /**  */
    private String deviceMac;
    /**  */
    private String deviceQrcode;
    /**  */
    private String deviceType;

    private String appId;
    private String productId;

    public void setDeviceId(String deviceId)
    {
        this.deviceId = deviceId;
    }

    public String getDeviceId()
    {
        return deviceId;
    }
    public void setDeviceMac(String deviceMac)
    {
        this.deviceMac = deviceMac;
    }

    public String getDeviceMac()
    {
        return deviceMac;
    }
    public void setDeviceQrcode(String deviceQrcode)
    {
        this.deviceQrcode = deviceQrcode;
    }

    public String getDeviceQrcode()
    {
        return deviceQrcode;
    }
    public void setDeviceType(String deviceType)
    {
        this.deviceType = deviceType;
    }

    public String getDeviceType()
    {
        return deviceType;
    }

    public String getAppId() {
        return appId;
    }

    public void setAppId(String appId) {
        this.appId = appId;
    }

    public String getProductId() {
        return productId;
    }

    public void setProductId(String productId) {
        this.productId = productId;
    }

    @Override
    public String toString() {
        return new ToStringBuilder(this,ToStringStyle.MULTI_LINE_STYLE)
                .append("deviceId", getDeviceId())
                .append("deviceMac", getDeviceMac())
                .append("deviceQrcode", getDeviceQrcode())
                .append("deviceType", getDeviceType())
                .append("appId",getAppId())
                .append("product",getProductId())
                .toString();
    }
}