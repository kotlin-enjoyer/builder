#include <jni.h>
#include <map>
#include <string>
#include <iomanip>
#include <sstream>
#include <cctype>
#include <iostream>

std::map<std::string, std::string> getCppMapFromJavaMap(
        JNIEnv *env,
        jobject &map
) {
    std::map<std::string, std::string> cppMap;
    jclass mapClass = env->GetObjectClass(map);
    jmethodID entrySetMethod = env->GetMethodID(mapClass, "entrySet", "()Ljava/util/Set;");
    jobject entrySet = env->CallObjectMethod(map, entrySetMethod);
    jclass setClass = env->GetObjectClass(entrySet);
    jmethodID iteratorMethod = env->GetMethodID(setClass, "iterator", "()Ljava/util/Iterator;");
    jobject iterator = env->CallObjectMethod(entrySet, iteratorMethod);
    jclass iteratorClass = env->GetObjectClass(iterator);
    jmethodID hasNextMethod = env->GetMethodID(iteratorClass, "hasNext", "()Z");
    jmethodID nextMethod = env->GetMethodID(iteratorClass, "next", "()Ljava/lang/Object;");
    while (env->CallBooleanMethod(iterator, hasNextMethod)) {
        jobject entry = env->CallObjectMethod(iterator, nextMethod);
        jclass entryClass = env->GetObjectClass(entry);
        jmethodID getKeyMethod = env->GetMethodID(entryClass, "getKey", "()Ljava/lang/Object;");
        jmethodID getValueMethod = env->GetMethodID(entryClass, "getValue", "()Ljava/lang/Object;");
        auto jKey = static_cast<jstring>(env->CallObjectMethod(entry, getKeyMethod));
        auto jValue = static_cast<jstring>(env->CallObjectMethod(entry, getValueMethod));
        const char *key = env->GetStringUTFChars(jKey, nullptr);
        const char *value = env->GetStringUTFChars(jValue, nullptr);
        // Populate the C++ map
        cppMap[key] = value;
        env->ReleaseStringUTFChars(jKey, key);
        env->ReleaseStringUTFChars(jValue, value);
        env->DeleteLocalRef(entry);
    }
    return cppMap;
}

std::string urlEncode(const std::string &inputString) {
    std::ostringstream encodedStream;
    for (char c: inputString) {
        if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            encodedStream << c;
        } else {
            encodedStream << '%' << std::uppercase << std::setw(2) << std::setfill('0') << std::hex
                          << static_cast<int>(static_cast<unsigned char>(c));
        }
    }
    return encodedStream.str();
}

void processCampaign(std::map<std::string, std::string> &map) {
    if (map.count("campaign_1") && map.count("campaign_2")) {
        std::string selectedCampaign = (!map["campaign_1"].empty() &&
                                        map["campaign_1"] != "null" &&
                                        map["campaign_1"] != "None") ?
                                       map["campaign_1"] : map["campaign_2"];
        map["campaign"] = selectedCampaign;
        map.erase("campaign_1");
        map.erase("campaign_2");
    }

    if (map.count("campaign")) {
        size_t pos;
        int i = 0;
        std::string camp = map["campaign"];
        std::string prefix = "myapp://";
        if (camp.compare(0, prefix.length(), prefix) == 0) camp.erase(0, prefix.length());

        while ((pos = camp.find('_')) != std::string::npos || !camp.empty()) {
            std::string token = (pos != std::string::npos) ? camp.substr(0, pos) : camp;
            camp.erase(0, (pos != std::string::npos) ? pos + 1 : pos);

            std::string name = i == 1 ? "push" : ("sub" + std::to_string(i == 0 ? 1 : i));
            map[name] = token;
            ++i;
        }
        while (i < 11) {
            std::string name = i == 1 ? "push" : ("sub" + std::to_string(i == 0 ? 1 : i));
            map[name] = i < 2 ? "null" : (i == 10 ? "firstOpen" : "");
            ++i;
        }
    }
    map["sub10"] = "firstOpen";
    map["notId"] = "null";
}

std::string convertMapToQueryString(
        const std::string &queryString,
        const std::map<std::string, std::string> &map
) {
    std::string res = std::string(queryString);
    for (const auto &entry: map) {
        res += entry.first + "=" + urlEncode(entry.second) + "&";
    }
    return res;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_kotlinenjoer_builder_Builder_buildQuery(
        JNIEnv *env,
        jobject /* this */,
        jstring jDomain,
        jobject map
) {
    std::map<std::string, std::string> cppMap = getCppMapFromJavaMap(env, map);
    const char *domain = env->GetStringUTFChars(jDomain, nullptr);
    processCampaign(cppMap);
    std::string result = convertMapToQueryString(domain, cppMap);
    if (!cppMap.empty()) {
        result.pop_back();
    }
    env->ReleaseStringUTFChars(jDomain, domain);
    return env->NewStringUTF(result.c_str());
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_kotlinenjoer_builder_Builder_replaceParamValue(
        JNIEnv *env,
        jobject /* this */,
        jstring jQuery,
        jstring jParamName,
        jstring jParamValue
) {
    const char *query = env->GetStringUTFChars(jQuery, nullptr);
    const char *paramName = env->GetStringUTFChars(jParamName, nullptr);
    const char *paramValue = env->GetStringUTFChars(jParamValue, nullptr);

    std::istringstream iss(query);
    std::string path, uQuery;
    std::getline(iss, path, '?');
    std::getline(iss, uQuery);

    std::map<std::string, std::string> paramMap;
    std::istringstream paramStream(uQuery);
    std::string paramPair;
    while (std::getline(paramStream, paramPair, '&')) {
        size_t equalsPos = paramPair.find('=');
        if (equalsPos != std::string::npos) {
            paramMap[paramPair.substr(1, equalsPos)] = paramPair.substr(equalsPos + 1);
        }
    }
    paramMap[paramName] = paramValue;

    std::ostringstream newUrl;
    newUrl << path << "?";
    for (const auto &entry: paramMap) {
        newUrl << entry.first << "=" << entry.second << "&";
    }

    std::string result = newUrl.str();
    if (!paramMap.empty()) {
        result.pop_back();
    }

    env->ReleaseStringUTFChars(jQuery, query);
    env->ReleaseStringUTFChars(jParamName, paramName);
    env->ReleaseStringUTFChars(jParamValue, paramValue);

    return env->NewStringUTF(result.c_str());
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_kotlinenjoer_builder_Builder_getParamValue(
        JNIEnv *env,
        jobject /* this */,
        jstring jQuery,
        jstring jParamName
) {
    const char *query = env->GetStringUTFChars(jQuery, nullptr);
    const char *paramName = env->GetStringUTFChars(jParamName, nullptr);

    std::istringstream iss(query);
    std::string path, uQuery;
    std::getline(iss, path, '?');
    std::getline(iss, uQuery);

    std::map<std::string, std::string> paramMap;
    std::istringstream paramStream(uQuery);
    std::string paramPair;
    while (std::getline(paramStream, paramPair, '&')) {
        size_t equalsPos = paramPair.find('=');
        if (equalsPos != std::string::npos) {
            paramMap[paramPair.substr(0, equalsPos)] = paramPair.substr(equalsPos + 1);
        }
    }
    std::string res = paramMap.count(paramName) ? paramMap[paramName] : "";

    env->ReleaseStringUTFChars(jQuery, query);
    env->ReleaseStringUTFChars(jParamName, paramName);

    return env->NewStringUTF(res.c_str());
}
