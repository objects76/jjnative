import addon, { bigintFromHandle } from "../lib/index";

const logger = require("./Logger.js");
const getMethods = logger.getMethods;
const getStaticMethods = logger.getStaticMethods;

function test_main() {
  logger.log("node version = ", process.versions.node);
  logger.log("node_env=", process.env.NODE_ENV);

  let buf8 = Buffer.from([0xff, 0xff, 0xff, 0xff, 0x00, 0xff, 0x00, 0xab]);
  let buf4 = Buffer.from([0x00, 0x00, 0x00, 0xff]);
  logger.log(buf8, "0x" + bigintFromHandle(buf8).toString(16));
  logger.log(buf4, "0x" + bigintFromHandle(buf4).toString(16));

  try {
    // startKeyMonitor with invalid window handle.
    addon.startKeyMonitor(BigInt(1));
  } catch (error) {
    console.error("startKeyMonitor exception: ", error);
  }
}
//test_main();

function dumpObject(obj: Object) {
  logger.log(obj.constructor.name);
  console.log("Object.values()");
  console.log(Object.values(obj));
  console.log(Object.getPrototypeOf(obj));
  console.log(Object.getOwnPropertyNames(obj));

  logger.log("ctor methods:", getStaticMethods(obj));
  logger.log("call static method:");

  // let properties = new Set();
  // for (let cur = obj.constructor; cur; cur = Object.getPrototypeOf(cur)) {
  //   //console.log(Object.getOwnPropertyNames(cur));
  //   Object.getOwnPropertyNames(cur).map(item => properties.add(item));
  // }

  // logger.log('static method from instance');
  // logger.log('static dump=', typeof (obj.constructor as any)['dumpx']);
  // logger.log((obj.constructor as any).toString());

  // logger.log('call static method from instance');
  // logger.log((obj.constructor as any).dump(obj));

  // logger.log([...properties.keys()]);
  //return [...properties.keys()].filter((item) => typeof obj[item] === "function");
}

async function test_native() {

  // logger.log("result=", await addon.getPrimeAsync());
  // logger.log(addon.getPrimeSync());

  const obj = new addon.NativeAddon(
    () => { console.log('Hi! I\'m a JSRef function!') },
    () => { console.log('Hi! I\'m a JS function!') }
  );
  //console.log(getMethods(obj));

  dumpObject(obj);
  return;

  addon.dumpNativeAddon(obj);

  const ITERATIONS = 5
  for (let i = 0; i < ITERATIONS; i++) {
    logger.log(obj.tryCallByStoredReference());
  }


  try {
    //logger.log(obj.tryCallByStoredFunction());
  } catch (err) {
    console.error(err);
    console.error('This code crashes because JSFn is valid only inside the constructor function.')
    console.error('The lifespan of the JSFn function is limited to the execution of the constructor function.')
    console.error('After that, the function stored in JSFn is ready to be garbage collected and it cannot be used anymore.')
  }
}

test_native();
