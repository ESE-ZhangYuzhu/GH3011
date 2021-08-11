package me.chanjar.weixin.mp.bean.message;

import com.google.gson.FieldNamingPolicy;
import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import com.thoughtworks.xstream.annotations.XStreamAlias;
import com.thoughtworks.xstream.annotations.XStreamConverter;
import lombok.Data;
import lombok.extern.slf4j.Slf4j;
import me.chanjar.weixin.common.api.WxConsts;
import me.chanjar.weixin.common.util.xml.XStreamCDataConverter;
import me.chanjar.weixin.mp.api.WxMpConfigStorage;
import me.chanjar.weixin.mp.util.crypto.WxMpCryptUtil;
import me.chanjar.weixin.mp.util.xml.XStreamTransformer;
import org.apache.commons.io.IOUtils;
import org.apache.commons.lang3.builder.ToStringBuilder;
import org.apache.commons.lang3.builder.ToStringStyle;

import java.io.IOException;
import java.io.InputStream;
import java.io.Serializable;

/**
 * <pre>
 * 微信推送过来的消息，xml格式.
 * 部分未注释的字段的解释请查阅相关微信开发文档：
 * <a href="http://mp.weixin.qq.com/wiki?t=resource/res_main&id=mp1421140453&token=&lang=zh_CN">接收普通消息</a>
 * <a href="http://mp.weixin.qq.com/wiki?t=resource/res_main&id=mp1421140454&token=&lang=zh_CN">接收事件推送</a>
 * </pre>
 *
 * @author chanjarster
 */
@Data
@Slf4j
public class WxMpJsonMessage implements Serializable {
  private static final long serialVersionUID = -3586245291677274914L;

  ///////////////////////
  // 以下都是微信推送过来的消息的xml的element所对应的属性
  ///////////////////////

  private String toUser;

  private String fromUser;


  private Long createTime;

  private String msgType;

  private String content;

  private Long menuId;

  private Long msgId;

  private String picUrl;

  private String mediaId;

  private String format;

  private String thumbMediaId;

  private Double locationX;

  private Double locationY;

  private Double scale;

  private String label;

  private String title;

  private String description;

  private String url;

  private String event;

  private String eventKey;

  private String ticket;

  private Double latitude;

  private Double longitude;

  private Double precision;

  private String recognition;

  ///////////////////////////////////////
  // 群发消息返回的结果
  ///////////////////////////////////////
  /**
   * 群发的结果.
   */
  private String status;
  /**
   * group_id下粉丝数；或者openid_list中的粉丝数.
   */
  private Integer totalCount;
  /**
   * 过滤（过滤是指特定地区、性别的过滤、用户设置拒收的过滤，用户接收已超4条的过滤）后，准备发送的粉丝数.
   * 原则上，filterCount = sentCount + errorCount
   */
  private Integer filterCount;
  /**
   * 发送成功的粉丝数.
   */
  private Integer sentCount;
  /**
   * 发送失败的粉丝数.
   */
  private Integer errorCount;

  ///////////////////////////////////////
  // 客服会话管理相关事件推送
  ///////////////////////////////////////
  /**
   * 创建或关闭客服会话时的客服帐号.
   */
  private String kfAccount;
  /**
   * 转接客服会话时的转入客服帐号.
   */
  private String toKfAccount;
  /**
   * 转接客服会话时的转出客服帐号.
   */
  private String fromKfAccount;

  ///////////////////////////////////////
  // 卡券相关事件推送
  ///////////////////////////////////////

  private String cardId;

  private String friendUserName;

  /**
   * 是否为转赠，1代表是，0代表否.
   */
  private Integer isGiveByFriend;

  private String userCardCode;

  private String oldUserCardCode;

  private Integer outerId;

  /**
   * 用户删除会员卡后可重新找回，当用户本次操作为找回时，该值为1，否则为0.
   */
  private String isRestoreMemberCard;

  /**
   * <pre>
   * 领取场景值，用于领取渠道数据统计。可在生成二维码接口及添加Addcard接口中自定义该字段的字符串值.
   * 核销卡券时：开发者发起核销时传入的自定义参数，用于进行核销渠道统计
   * 另外：
   * 官网文档中，微信卡券>>卡券事件推送>>2.7 进入会员卡事件推送 user_view_card
   * OuterStr：商户自定义二维码渠道参数，用于标识本次扫码打开会员卡来源来自于某个渠道值的二维码
   * </pre>
   */
  private String outerStr;

  /**
   * 是否转赠退回，0代表不是，1代表是.
   */
  private String isReturnBack;

  /**
   * 是否是群转赠，0代表不是，1代表是.
   */
  private String isChatRoom;

  /**
   * 核销来源.
   * 支持开发者统计API核销（FROM_API）、公众平台核销（FROM_MP）、卡券商户助手核销（FROM_MOBILE_HELPER）（核销员微信号）
   */
  private String consumeSource;

  /**
   * 门店名称.
   * 当前卡券核销的门店名称（只有通过自助核销和买单核销时才会出现该字段）
   */
  private String locationName;

  /**
   * 核销该卡券核销员的openid（只有通过卡券商户助手核销时才会出现）.
   */
  private String staffOpenId;

  /**
   * 自助核销时，用户输入的验证码.
   */
  private String verifyCode;

  /**
   * 自助核销时，用户输入的备注金额.
   */
  private String remarkAmount;

  /**
   * <pre>
   * 官网文档中，微信卡券>>卡券事件推送>>2.10 库存报警事件card_sku_remind
   * Detail：报警详细信息.
   * </pre>
   */
  private String detail;

  /**
   * <pre>
   * 官网文档中，微信卡券>>卡券事件推送>>2.9 会员卡内容更新事件 update_member_card
   * ModifyBonus：变动的积分值.
   * </pre>
   */
  private String modifyBonus;

  /**
   * <pre>
   * 官网文档中，微信卡券>>卡券事件推送>>2.9 会员卡内容更新事件 update_member_card
   * ModifyBalance：变动的余额值.
   * </pre>
   */
  private String modifyBalance;

  /**
   * <pre>
   * 官网文档中，微信卡券>>卡券事件推送>>2.6 买单事件推送 User_pay_from_pay_cell
   * TransId：微信支付交易订单号（只有使用买单功能核销的卡券才会出现）.
   * </pre>
   */
  private String transId;

  /**
   * <pre>
   * 官网文档中，微信卡券>>卡券事件推送>>2.6 买单事件推送 User_pay_from_pay_cell
   * LocationId：门店ID，当前卡券核销的门店ID（只有通过卡券商户助手和买单核销时才会出现）
   * </pre>
   */
  private String locationId;

  /**
   * <pre>
   * 官网文档中，微信卡券>>卡券事件推送>>2.6 买单事件推送 User_pay_from_pay_cell
   * Fee：实付金额，单位为分
   * </pre>
   */
  private String fee;

  /**
   * <pre>
   * 官网文档中，微信卡券>>卡券事件推送>>2.6 买单事件推送 User_pay_from_pay_cell
   * OriginalFee：应付金额，单位为分
   * </pre>
   */
  private String originalFee;

  private ScanCodeInfo scanCodeInfo = new ScanCodeInfo();

  private SendPicsInfo sendPicsInfo = new SendPicsInfo();

  private SendLocationInfo sendLocationInfo = new SendLocationInfo();

  /**
   * 审核不通过原因
   */
  private String refuseReason;

  /**
   * 是否为朋友推荐，0代表否，1代表是
   */
  private String isRecommendByFriend;

  /**
   * 购买券点时，实际支付成功的时间
   */
  private String payFinishTime;

  /**
   * 购买券点时，支付二维码的生成时间
   */
  private String createOrderTime;

  /**
   * 购买券点时，支付二维码的生成时间
   */
  private String desc;

  /**
   * 剩余免费券点数量
   */
  private String freeCoinCount;

  /**
   * 剩余付费券点数量
   */
  private String payCoinCount;

  /**
   * 本次变动的免费券点数量
   */
  private String refundFreeCoinCount;

  /**
   * 本次变动的付费券点数量
   */
  private String refundPayCoinCount;

  /**
   * <pre>
   *    所要拉取的订单类型 ORDER_TYPE_SYS_ADD 平台赠送券点 ORDER_TYPE_WXPAY 充值券点 ORDER_TYPE_REFUND 库存未使用回退券点
   *    ORDER_TYPE_REDUCE 券点兑换库存 ORDER_TYPE_SYS_REDUCE 平台扣减
   * </pre>
   */
  private String orderType;

  /**
   * 系统备注，说明此次变动的缘由，如开通账户奖励、门店奖励、核销奖励以及充值、扣减。
   */
  private String memo;

  /**
   * 所开发票的详情
   */
  private String receiptInfo;


  ///////////////////////////////////////
  // 门店审核事件推送
  ///////////////////////////////////////
  /**
   * 商户自己内部ID，即字段中的sid.
   */
  private String storeUniqId;

  /**
   * 微信的门店ID，微信内门店唯一标示ID.
   */
  private String poiId;

  /**
   * 审核结果，成功succ 或失败fail.
   */
  private String result;

  /**
   * 成功的通知信息，或审核失败的驳回理由.
   */
  private String msg;

  ///////////////////////////////////////
  // 微信认证事件推送
  ///////////////////////////////////////
  /**
   * 资质认证成功/名称认证成功: 有效期 (整形)，指的是时间戳，将于该时间戳认证过期.
   * 年审通知: 有效期 (整形)，指的是时间戳，将于该时间戳认证过期，需尽快年审
   * 认证过期失效通知: 有效期 (整形)，指的是时间戳，表示已于该时间戳认证过期，需要重新发起微信认证
   */
  private Long expiredTime;
  /**
   * 失败发生时间 (整形)，时间戳.
   */
  private Long failTime;
  /**
   * 认证失败的原因.
   */
  private String failReason;

  ///////////////////////////////////////
  // 微信小店 6.1订单付款通知
  ///////////////////////////////////////
  /**
   * 订单ID.
   */
  private String orderId;

  /**
   * 订单状态.
   */
  private String orderStatus;

  /**
   * 商品ID.
   */
  private String productId;

  /**
   * 商品SKU信息.
   */
  private String skuInfo;

  ///////////////////////////////////////
  // 微信硬件平台相关事件推送
  ///////////////////////////////////////
  /**
   * 设备类型.
   * 目前为"公众账号原始ID"
   */
  private String deviceType;

  /**
   * 设备ID.
   * 第三方提供
   */
  private String deviceId;

  /**
   * 微信用户账号的OpenID.
   */
  private String openId;

  private HardWare hardWare = new HardWare();

  /**
   * 请求类型.
   * 0：退订设备状态；
   * 1：心跳；（心跳的处理方式跟订阅一样）
   * 2：订阅设备状态
   */
  private Integer opType;

  /**
   * 设备状态.
   * 0：未连接；1：已连接
   */
  private Integer deviceStatus;

  private String sessionID;

  public static void  main(String[] args){
    String origin ="tfQ4v1dDTeJGivGLKYuUh3LKimB+n5FZGJarQVYH7K05RARZR1ktzbHRT4xCCVBDK4v4eo/+aClP6pcG6OkquNz1r6CebvcgXgEMB0PjM9sf01GhH2E04FADoe+4MoKtkWtfmZyO+49JlODGI/bVrsFi8QuqYbklmz+y2KjUWJctppMzIScodI4Sy33LDPGpMKGyzGWwqvOv94tFzAE/FBhqXqzOc0QZZy8Zbs/TqcQzf0jm3d5TqVu2S10kwgngS45hsiye+SE5AKZps5mIwSdstzF5J+UWB1J4JBZmmGmGivuHwX1grFv2owUtuubvii/U89gT1z5dZr1sBVflEs3BO0kUVMzScm+xFMqiWOWx16ruGzSIyafdba4alPnz";


  }

  public static WxMpJsonMessage fromJson(String jsonBody) {
    //修改微信变态的消息内容格式，方便解析
    Gson gson = new GsonBuilder().setFieldNamingPolicy(FieldNamingPolicy.LOWER_CASE_WITH_UNDERSCORES).create();
    return gson.fromJson(jsonBody,WxMpJsonMessage.class);
  }


  /**
   * 从加密字符串转换.
   *
   * @param encryptedJson      密文
   * @param wxMpConfigStorage 配置存储器对象
   * @param timestamp         时间戳
   * @param nonce             随机串
   * @param msgSignature      签名串
   */
  public static WxMpJsonMessage fromEncryptedJson(String encryptedJson, WxMpConfigStorage wxMpConfigStorage,
                                                 String timestamp, String nonce, String msgSignature) {
    WxMpCryptUtil cryptUtil = new WxMpCryptUtil(wxMpConfigStorage);
    String jsonText=encryptedJson.substring("\"encrypt\":".length());
    log.debug("原始json消息内容：{}", jsonText);
    log.info("原始json消息内容：{}", jsonText);
    String plainText = cryptUtil.decrypt(jsonText);
    log.debug("解密后的原始json消息内容：{}", plainText);
    log.info("解密后的原始json消息内容：{}", plainText);
    return fromJson(plainText);
  }

  /**
   * <pre>
   * 当接受用户消息时，可能会获得以下值：
   * {@link WxConsts.XmlMsgType#TEXT}
   * {@link WxConsts.XmlMsgType#IMAGE}
   * {@link WxConsts.XmlMsgType#VOICE}
   * {@link WxConsts.XmlMsgType#VIDEO}
   * {@link WxConsts.XmlMsgType#LOCATION}
   * {@link WxConsts.XmlMsgType#LINK}
   * {@link WxConsts.XmlMsgType#EVENT}
   * </pre>
   */
  public String getMsgType() {
    return this.msgType;
  }

  /**
   * <pre>
   * 当发送消息的时候使用：
   * {@link WxConsts.XmlMsgType#TEXT}
   * {@link WxConsts.XmlMsgType#IMAGE}
   * {@link WxConsts.XmlMsgType#VOICE}
   * {@link WxConsts.XmlMsgType#VIDEO}
   * {@link WxConsts.XmlMsgType#NEWS}
   * {@link WxConsts.XmlMsgType#MUSIC}
   * </pre>
   */
  public void setMsgType(String msgType) {
    this.msgType = msgType;
  }

  @Override
  public String toString() {
    return ToStringBuilder.reflectionToString(this, ToStringStyle.JSON_STYLE);
  }

}
