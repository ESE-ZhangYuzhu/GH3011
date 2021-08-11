package com.goodix;

import org.mybatis.spring.annotation.MapperScan;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.boot.autoconfigure.jdbc.DataSourceAutoConfiguration;

import java.util.Calendar;
import java.util.TimeZone;

/**
 * 启动程序
 * 
 *
 */
@SpringBootApplication(exclude = { DataSourceAutoConfiguration.class })
@MapperScan("com.goodix.project.*.*.mapper")
public class WeiXinApplication
{
    public static void main(String[] args)
    {
        System.setProperty("spring.devtools.restart.enabled", "true");
        SpringApplication.run(WeiXinApplication.class, args);
        System.out.println("启动成功\n");
    }
}