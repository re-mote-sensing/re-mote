import i18n from "i18next";
import { initReactI18next } from "react-i18next";
import LanguageDetector from 'i18next-browser-languagedetector';

// The translations
const resources = {
  en: {
    translation: {
      "title": "Turtle Tracker Online Monitoring",
      "history": "History",
      "tracker history":"Tracker History",
      "settings": "Settings",
      "notice-in-history":"Click on the tracker title to find it in map",
      "tracker":"Tracker",
      "turtle tracker":"Turtle Tracker",
      "trace route":"Trace Route",
      "select language": "",
      "bluetooth": "Bluetooth",
      "bluetooth connect": "Connect to Gateway via BLE",
      "browser not supported": "Your browser is not supported",
      "Department of Computing and Software": "Department of Computing and Software",
      "McMaster University": "McMaster University",
    }
  },
  fr: {
    translation: {
      "title": "Surveillance en ligne de Turtle Tracker",
      "history": "Histoire",
      "tracker history":"historique du suivi",
      "settings": "Paramètres",
      "notice-in-history":"Cliquez sur le titre du tracker pour le retrouver sur la carte",
      "tracker":"Traqueur",
      "turtle tracker":"Turtle Tracker",
      "trace route":"Tracer l'itinéraire",
      "select language": " | Choisir la langue",
      "bluetooth": "Bluetooth",
      "bluetooth connect": "Connectez-vous à la passerelle via BLE",
      "browser not supported": "Ton navigateur n'est pas supporté",
      "Department of Computing and Software": "Département d'informatique et de logiciels",
      "McMaster University": "Université McMaster",
    }
  },
  // de: {
  //   translation: {
  //     "title": "Turtle Tracker Online-Überwachung",
  //     "history": "Geschichte",
  //     "tracker history":"Tracker-Verlauf",
  //     "settings": "Einstellungen",
  //     "notice-in-history":"Klicken Sie auf den Titel des Trackers, um ihn auf der Karte zu finden",
  //     "tracker":"Tracker",
  //     "turtle tracker":"Schildkröten-Tracker",
  //     "trace route":"Route verfolgen",
  //     "select language": " | Sprache auswählen",
  //     "bluetooth": "Bluetooth",
  //     "bluetooth connect": "Über BLE mit Gateway verbinden",
  //     "browser not supported": "Ihr Browser wird nicht unterstützt",
  //     "Department of Computing and Software": "Fakultät für Informatik und Software",
  //     "McMaster University": "McMaster University",
  //   }
  // },
  // zh: {
  //   translation: {
  //     "title": "Turtle Tracker 在线监测",
  //     "history": "历史",
  //     "tracker history":"追踪器历史",
  //     "settings": "设置",
  //     "notice-in-history":"点击追踪器标题以在地图中显示",
  //     "tracker":"追踪器",
  //     "turtle tracker":"Turtle 追踪器",
  //     "trace route":"追踪路径",
  //     "select language": " | 选择语言",
  //     "bluetooth": "蓝牙",
  //     "bluetooth connect": "通过蓝牙连接网关",
  //     "browser not supported": "您的浏览器不受支持",
  //     "Department of Computing and Software": "计算与软件系",
  //     "McMaster University": "麦克马斯特大学",
  //   }
  // }
};

i18n
  .use(initReactI18next) // passes i18n down to react-i18next
  .use(LanguageDetector) // Auto Detector Language
  .init({
    resources,
    fallbackLng: "en",
    interpolation: {
      escapeValue: false // react already safes from xss
    }
  });

  export default i18n;