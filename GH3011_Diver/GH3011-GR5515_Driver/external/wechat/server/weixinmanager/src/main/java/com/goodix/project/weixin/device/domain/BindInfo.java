package com.goodix.project.weixin.device.domain;

import com.goodix.framework.web.domain.BaseEntity;
import org.apache.commons.lang3.builder.ToStringBuilder;
import org.apache.commons.lang3.builder.ToStringStyle;

import java.util.Date;

/**
 * 操作状态
 *
 * @author goodix
 */
public class BindInfo extends BaseEntity
{
    private static final long serialVersionUID = 1L;

    /** 设备ID */
    private String deviceId;
    /** 用户微信ID */
    private String openId;
    /** 1绑定,2解绑 */
    private Integer status;
    /** 创建人 */
    private String createBy;
    /** 绑定时间 */
    private Date createTime;
    /** 修改入 */
    private String updateBy;
    /** 修改时间 */
    private Date updateTime;
    /** 备注 */
    private String remark;

    private DeviceInfo deviceInfo;

    public void setDeviceId(String deviceId)
    {
        this.deviceId = deviceId;
    }

    public String getDeviceId()
    {
        return deviceId;
    }
    public void setOpenId(String openId)
    {
        this.openId = openId;
    }

    public String getOpenId()
    {
        return openId;
    }
    public void setStatus(Integer status)
    {
        this.status = status;
    }

    public Integer getStatus()
    {
        return status;
    }
    public void setCreateBy(String createBy)
    {
        this.createBy = createBy;
    }

    public String getCreateBy()
    {
        return createBy;
    }
    public void setCreateTime(Date createTime)
    {
        this.createTime = createTime;
    }

    public Date getCreateTime()
    {
        return createTime;
    }
    public void setUpdateBy(String updateBy)
    {
        this.updateBy = updateBy;
    }

    public String getUpdateBy()
    {
        return updateBy;
    }
    public void setUpdateTime(Date updateTime)
    {
        this.updateTime = updateTime;
    }

    public Date getUpdateTime()
    {
        return updateTime;
    }
    public void setRemark(String remark)
    {
        this.remark = remark;
    }

    public String getRemark()
    {
        return remark;
    }

    public DeviceInfo getDeviceInfo() {
        return deviceInfo;
    }

    public void setDeviceInfo(DeviceInfo deviceInfo) {
        this.deviceInfo = deviceInfo;
    }

    public String toString() {
        return new ToStringBuilder(this,ToStringStyle.MULTI_LINE_STYLE)
                .append("deviceId", getDeviceId())
                .append("openId", getOpenId())
                .append("status", getStatus())
                .append("createBy", getCreateBy())
                .append("createTime", getCreateTime())
                .append("updateBy", getUpdateBy())
                .append("updateTime", getUpdateTime())
                .append("remark", getRemark())
                .toString();
    }
}
