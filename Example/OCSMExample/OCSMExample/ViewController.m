//
//  ViewController.m
//  OCSMExample
//
//  Created by lxm on 2016/12/26.
//  Copyright © 2016年 lxm. All rights reserved.
//

#import "ViewController.h"

#define OCSMSTRING2 MUDECODESTRING("8153913A63CC8ACC1C091FF49C25000CE95CF85966A1C9E7")

@interface ViewController ()
@property (weak, nonatomic) IBOutlet UILabel *label1;
@property (weak, nonatomic) IBOutlet UILabel *label2;
@property (weak, nonatomic) IBOutlet UIButton *button1;
@property (weak, nonatomic) IBOutlet UIButton *button2;

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view, typically from a nib.
    self.label1.text = OCSMSTRING1;
    self.label2.text = OCSMSTRING2;
//    [self.button1 setTitle:@"" forState:UIControlStateNormal];
//    [self.button2 setTitle:@"" forState:UIControlStateNormal];
//    [self.button1 addTarget:self action:NSSelectorFromString(@"actionA") forControlEvents:UIControlEventTouchUpInside];
    [self.button1 setTitle:MUDECODESTRING("8F579E3640FD86CA380A31C39E39190CDA77F5787D") forState:UIControlStateNormal];
    [self.button2 setTitle:MUDECODESTRING("826E833653F484F43D0839FA902C3902C147F57A53") forState:UIControlStateNormal];
    [self.button1 addTarget:self action:NSSelectorFromString(MUDECODESTRING("06BF75BAA73A22")) forControlEvents:UIControlEventTouchUpInside];
    [self.button1 addTarget:self action:@selector(actionB) forControlEvents:UIControlEventTouchUpInside];
}

- (void)actionA{
    NSLog(@"点击按钮一 ： %@",MUDECODESTRING("82558C3A55F684CB010A19F09C033F05CB40F74B49A0C8ED73682F83DA287370E7198E75"));
}

- (void)actionB{
//    NSURL *url = [NSURL URLWithString:@"http://www.baidu.com"];
    NSURL *url = [NSURL URLWithString:MUDECODESTRING("0FA875A3F27B4C26F298B11718C4E29F59AF7EAE")];
    [NSURLConnection sendSynchronousRequest:[NSURLRequest requestWithURL:url] returningResponse:nil error:nil];
}


@end
