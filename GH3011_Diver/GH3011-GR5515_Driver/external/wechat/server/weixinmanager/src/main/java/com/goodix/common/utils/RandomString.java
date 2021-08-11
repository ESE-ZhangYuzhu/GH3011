package com.goodix.common.utils;

import java.security.SecureRandom;
import java.util.Random;

/**
 * 操作状态
 *
 * @author goodix
 */
public class RandomString {
    public static final String SOURCES =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890";

    public static void main(String[] args) {
        RandomString rs = new RandomString();
        System.out.println(rs.generateString(new Random(),SOURCES,10));
        System.out.println(rs.generateString(new Random(),SOURCES,10));
        System.out.println(rs.generateString(new SecureRandom(),SOURCES,15));
        System.out.println(rs.generateString(new SecureRandom(),SOURCES,15));
    }

    /**
     * Generate a random string.
     *
     * @param random the random number generator.
     * @param characters the characters for generating string.
     * @param length the length of the generated string.
     * @return
     */
    public String generateString(Random random,String characters,int length) {
        char[] text = new char[length];
        for (int i = 0; i < length; i++) {
            text[i] = characters.charAt(random.nextInt(characters.length()));
        }
        return new String(text);
    }
}
