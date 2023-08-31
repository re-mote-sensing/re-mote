import i18n from "i18next";
import { initReactI18next } from "react-i18next";
import LanguageDetector from 'i18next-browser-languagedetector';

i18n
  .use(initReactI18next) // passes i18n down to react-i18next
  .use(LanguageDetector) // Auto Detector Language
  .init({
    resources: {
      en: {
        translation: {
          "dashboard": {
            "map": "Map",
            "settings": "Settings",
            "info": "About",
            "configmode": "Config"
          },
          "about": {
            "sixnations": "Located along the banks of the Grand River, Six Nations is the only reserve in North America where all six Haudenosaunee nations live together.",
            "macwater": "MacWater is a collaborative project involving various disciplines of Scientists and Engineers working together to tackle water quality problems.",
            "gwf": "Global Water Future is a joint effort by the University of Saskatchewan, University of Waterloo and McMaster University to manage water futures in the face of dramatically increasing risks.",
            "ohneganos": "Our program is made up of two main projects: Co-Creation of Indigenous Water Quality Tools and Ohneganos - Indigenous Ecological Knowledge, Training, and Co-Creation of Mixed-Method Tools.",
            "configmode": "Config End Node by browser, connect via Serial Port"
          },
          "map": {
            "select-type-location": "Select a node and sensor type"
          },
          "menu": {
            "location": "Node",
            "any": "Any",
            "sensor-type": "Probe"
          },
          "sensor-table": {
            "title": "Time Series Graph of {{type}} Sensor:",
            "time": "Time",
            "value": "Value [ {{label}} ]",
            "location": "Location - {{name}}",
            "no-date": "No data available for {{name}}"
          },
          "settings": {
            "select-language": "Select Language"
          },
          "gwf-tweets": "Tweets by GWFutures",
          "learn-more": "Learn More"
        }
      },
      fr: {
        translation: {
          "dashboard": {
            "map": "Carte",
            "settings": "Paramètres",
            "info": "À propos de"
          },
          "about": {
            "sixnations": "Situé le long des rives de la rivière Grand, Six Nations est la seule réserve en Amérique du Nord où les six nations Haudenosaunee vivent ensemble.",
            "macwater": "MacWater est un projet collaboratif impliquant diverses disciplines de scientifiques et d'ingénieurs travaillant ensemble pour s'attaquer aux problèmes de qualité de l'eau.",
            "gwf": "Global Water Future est un effort conjoint de l'Université de la Saskatchewan, de l'Université de Waterloo et de l'Université McMaster pour gérer l'avenir de l'eau face à l'augmentation spectaculaire des risques.",
            "ohneganos": "Notre programme est composé de deux projets principaux : Co-création d'outils autochtones sur la qualité de l'eau et Ohneganos - Connaissances écologiques autochtones, formation et co-création d'outils à méthodes mixtes."
          },
          "map": {
            "select-type-location": "Sélectionnez un type de capteur et un emplacement"
          },
          "menu": {
            "location": "Nœud",
            "any": "Tout",
            "sensor-type": "Capteur"
          },
          "sensor-table": {
            "title": "Graphique de la série chronologique du capteur de {{type}}:",
            "time": "Temps",
            "value": "Valeur [ {{label}} ]",
            "location": "Emplacement - {{name}}",
            "no-date": "Pas de données disponibles pour {{name}}"
          },
          "settings": {
            "select-language": "Select Language"
          },
          "gwf-tweets": "Tweets du GWFutures",
          "learn-more": "Apprendre encore plus"
        }
      },
      // de: {
      //   translation: {
      //     "dashboard": {
      //       "map": "Karte",
      //       "settings": "Einstellungen",
      //       "info": "Um"
      //     },
      //     "about": {
      //       "macwater": "MacWater ist ein Gemeinschaftsprojekt, an dem verschiedene Disziplinen von Wissenschaftlern und Ingenieuren zusammenarbeiten, um Probleme mit der Wasserqualität anzugehen.",
      //       "gwf": "Global Water Future ist eine gemeinsame Anstrengung der University of Saskatchewan, der University of Waterloo und der McMaster University, um die Wasserzukunft angesichts dramatisch zunehmender Risiken zu managen."    
      //     },
      //     "map": {
      //       "select-type-location": "Sensortyp und -position auswählen"
      //     },
      //     "menu": {
      //       "location": "Standort",
      //       "any": "Irgendein",
      //       "sensor-type": "Sensorart"
      //     },
      //     "sensor-table": {
      //       "title": "Zeitreihendiagramm von {{type}} Sensor:",
      //       "time": "Zeit",
      //       "value": "Wert [ {{label}} ]",
      //       "location": "Standort - {{name}}",
      //       "no-date": "Keine Daten verfügbar für {{name}}"
      //     },
      //     "settings": {
      //       "select-language": "Select Language"
      //     },
      //     "gwf-tweets": "Tweets von GWFutures",
      //     "learn-more": "Mehr erfahren"
      //   }
      // },
      // zh: {
      //   translation: {
      //     "dashboard": {
      //       "map": "地图",
      //       "settings": "设置",
      //       "info": "关于"
      //     },
      //     "about": {
      //       "macwater": "MacWater 是一个涉及不同学科的科学家和工程师共同努力解决水质问题合作项目。",
      //       "gwf": "Global Water Future 是萨斯喀彻温大学、滑铁卢大学和麦克马斯特大学共同致力于管理水在面临急剧增加的风险情况下的未来。"    
      //     },
      //     "map": {
      //       "select-type-location": "选择一个地点以及传感器类型"
      //     },
      //     "menu": {
      //       "location": "位置",
      //       "any": "所有",
      //       "sensor-type": "传感器种类"
      //     },
      //     "sensor-table": {
      //       "title": "传感器 {{type}} 的时间序列图:",
      //       "time": "时间",
      //       "value": "数值 [ {{label}} ]",
      //       "location": "地点 - {{name}}",
      //       "no-date": "{{name}} 没有数据"
      //     },
      //     "settings": {
      //       "select-language": "选择语言"
      //     },
      //     "gwf-tweets": "GWFutures 推特",
      //     "learn-more": "了解更多"
      //   }
      // },
    },
    fallbackLng: "en",
    interpolation: {
      escapeValue: false // react already safes from xss
    }
  });

  export default i18n;
