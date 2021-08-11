package com.goodix.framework.web.domain;

import io.swagger.models.auth.In;

import java.io.Serializable;

/**
 * jsticket获取结果信息
 *
 * @author goodix
 */
public class JsTicketResult implements Serializable{
    private String jsTicket;
    private String signature;
    private String noncestr;
    private Integer timestamp;

    public String getJsTicket() {
        return jsTicket;
    }

    public void setJsTicket(String jsTicket) {
        this.jsTicket = jsTicket;
    }

    public String getSignature() {
        return signature;
    }

    public void setSignature(String signature) {
        this.signature = signature;
    }

    public String getNoncestr() {
        return noncestr;
    }

    public void setNoncestr(String noncestr) {
        this.noncestr = noncestr;
    }

    public Integer getTimestamp() {
        return timestamp;
    }

    public void setTimestamp(Integer timestamp) {
        this.timestamp = timestamp;
    }
}
