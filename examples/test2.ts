import addon, { bigintFromHandle, NativeAddon } from "../lib/index";

const logger = require("./Logger.js");


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


async function test_native() {

  // passing array test
  {
    const array = new Int32Array([1, 2, 3, 4, 5, 6, 7, 8, 9, 10]);
    try {
      logger.log("ArrayBufferArgument=", addon.ArrayBufferArgument(array.buffer));

    } catch (error) {
      console.error(error);
    }
    return;

  }

  // promise test.
  logger.log("result=", await addon.getPrimeAsync());
  logger.log(addon.getPrimeSync());

  // class from addon.
  const obj = new addon.NativeAddon(
    () => { console.log('Hi! I\'m a JSRef function!') },
    () => { console.log('Hi! I\'m a JS function!') }
  );

  // static member funcion test.
  (addon.NativeAddon as any).staticMethod1(obj, "using staticMethod1");
  try {
    (addon.NativeAddon as any).staticMethod_Invalid(obj, "using staticMethod_Invalid");
  } catch (error) {
    console.error(error);
  }

  // member function test.
  logger.log(obj.tryCallByStoredReference());
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
